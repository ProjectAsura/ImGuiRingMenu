//-----------------------------------------------------------------------------
// File : ImGuiRingMenu.cpp
// Desc : ImGui Ring Menu.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiRingMenu.h"


namespace {

///////////////////////////////////////////////////////////////////////////////
// AnimState enum
///////////////////////////////////////////////////////////////////////////////
enum AnimState
{
    AnimNone,   //!< アニメなし.
    AnimIn,     //!< 開始アニメ.
    AnimOut     //!< 終了アニメ.
};

//-----------------------------------------------------------------------------
//      リングメニュー項目を描画します.
//-----------------------------------------------------------------------------
void DrawRingMenuItem(ImDrawList* dl, const ImGuiRingMenu::MenuItem& item, const ImVec2& pos, bool selected)
{
    float s = 64.0f;    // アイコンサイズ.

    // アイコンを描画.
    if (item.Icon != ImTextureID_Invalid)
    {
        dl->AddImage(item.Icon,
            ImVec2(pos.x - s * 0.5f, pos.y - s * 0.5f),
            ImVec2(pos.x + s * 0.5f, pos.y + s * 0.5f));
    }
    // 頭文字を描画.
    else
    {
        dl->AddRectFilled(
            ImVec2(pos.x - s * 0.5f, pos.y - s * 0.5f),
            ImVec2(pos.x + s * 0.5f, pos.y + s * 0.5f),
            (selected ? IM_COL32(255, 255, 255, 255) : IM_COL32(204, 204, 204, 255)),
            2.0f);
        const char capital[2] = { item.Label.c_str()[0], '\0'};
        auto textSize = ImGui::CalcTextSize(capital);
        dl->AddText(
            ImVec2(pos.x - textSize.x * 0.5f, pos.y - textSize.y * 0.5f),
            (selected ? IM_COL32(0, 0, 0, 255) : IM_COL32(255, 255, 255, 255)),
            capital);
    }

    // ラベル
    auto textSize = ImGui::CalcTextSize(item.Label.c_str());
        dl->AddText(
            ImVec2(pos.x - textSize.x * 0.5f, pos.y + s * 0.6f),
            (selected ? IM_COL32(255, 255, 0, 255) : IM_COL32(255, 255, 255, 255)),
            item.Label.c_str());
}

} // namespace


///////////////////////////////////////////////////////////////////////////////
// ImGuiRingMenu class
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//      コンストラクタです.
//-----------------------------------------------------------------------------
ImGuiRingMenu::ImGuiRingMenu()
{ /* DO_NOTHING */ }

//-----------------------------------------------------------------------------
//      デストラクタです.
//-----------------------------------------------------------------------------
ImGuiRingMenu::~ImGuiRingMenu()
{ Clear(); }

//-----------------------------------------------------------------------------
//      指定されたメニュー項目を追加します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::Add(const MenuItem& item)
{ m_Items.push_back(item); }

//-----------------------------------------------------------------------------
//      指定されたメニュー項目を削除します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::Remove(uint32_t index)
{ m_Items.erase(m_Items.begin() + index); }

//-----------------------------------------------------------------------------
//      メニュー項目を全削除します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::Clear()
{ m_Items.clear(); }

//-----------------------------------------------------------------------------
//      アニメーションを更新します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::Update(float deltaSec)
{
    // 開始アニメ.
    if (m_State == AnimIn)
    {
        m_AnimProgress = ImClamp(m_AnimProgress + deltaSec * m_AnimSpeed, 0.0f, 1.0f);
    }
    // 終了アニメ.
    else if (m_State == AnimOut)
    {
        m_AnimProgress = ImClamp(m_AnimProgress - deltaSec * m_AnimSpeed, 0.0f, 1.0f);
        // 終了判定.
        if (m_AnimProgress <= 1e-6f)
        {
            m_State        = AnimNone;
            m_AnimProgress = 0.0f;
        }
    }

    // 回転角を補間.
    m_CurrentAngle = ImLerp(m_CurrentAngle, m_TargetAngle, deltaSec * m_AnimSpeed);
 
    // 一定値以下になったら収束させる.
    if (fabsf(m_TargetAngle - m_CurrentAngle) <= 1e-6f)
    {
        m_CurrentAngle = m_TargetAngle;
    }
}

//-----------------------------------------------------------------------------
//      描画処理を行います.
//-----------------------------------------------------------------------------
bool ImGuiRingMenu::Draw(int& selectedIndex)
{
    // 項目数を取得.
    const int count = (int)m_Items.size();
    if (count == 0)
        return false;

    bool result = false;

    // 中心位置を求める.
    auto displaySize = ImGui::GetIO().DisplaySize;
    auto center = displaySize;
    center.x *= 0.5f;
    center.y *= 0.5f;

    // 縦・横の小さい方を半径として採用する.
    auto radius = (center.y < center.x) ? center.y : center.x;
    radius *= 0.5f;

    // 1項目の回転量.
    float rotateStep = (IM_PI * 2.0f) / float(count);

    // メニュー開始.
    if (ImGui::IsKeyPressed(ImGuiKey(m_KeyConfig.MenuStart)) && m_State == AnimNone)
    {
        m_State        = AnimIn;
        m_AnimProgress = 0.0f;
    }
    // メニュー終了.
    if (ImGui::IsKeyPressed(ImGuiKey(m_KeyConfig.MenuEnd)) && m_State == AnimIn)
    {
        m_State        = AnimOut;
        m_AnimProgress = 1.0f;
    }
    // メニュー決定.
    if (ImGui::IsKeyPressed(ImGuiKey(m_KeyConfig.Confirmation)) && m_State == AnimIn)
    {
        m_State        = AnimOut;
        m_AnimProgress = 1.0f;
        selectedIndex  = m_SelectedId;
        result = true;
    }

    // メニューが無効状態なら終了.
    if (m_State == AnimNone)
        return result;

    // 時計回りに回転.
    if (ImGui::IsKeyPressed(ImGuiKey(m_KeyConfig.CwRotate)))
    {
        m_TargetAngle -= rotateStep;
        m_SelectedId++;
    }
    // 反時計周りに回転.
    if (ImGui::IsKeyPressed(ImGuiKey(m_KeyConfig.CcwRotate)))
    {
        m_TargetAngle += rotateStep;
        m_SelectedId--;
    }

    // 番号をラップ.
    if (m_SelectedId >= count)
        m_SelectedId = 0;
    else if (m_SelectedId < 0)
        m_SelectedId = count - 1;

    auto r = radius * m_AnimProgress;   // 描画半径.
    auto startAngle = -IM_PI * 0.5f;    // 開始角度.
    auto endAngle   =  IM_PI * 1.5f;    // 終了角度.

    // 背景の描画リストを取得.
    auto dl = ImGui::GetBackgroundDrawList();

    // 各メニュー項目を描画.
    for(auto i=0; i<count; ++i)
    {
        auto t = float(i) / float(count);
        auto angle = startAngle + (endAngle - startAngle) * t;
        angle += m_CurrentAngle;

        ImVec2 pos(center.x + cosf(angle) * r,
                   center.y + sinf(angle) * r);

        int index = i % count;
        DrawRingMenuItem(dl, m_Items[index], pos, i == m_SelectedId);
    }

    // 選択されたら true を返す.
    return result;
}

//-----------------------------------------------------------------------------
//      キーコンフィグを設定します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::SetKeyConfig(const KeyConfig& value)
{ m_KeyConfig = value; }

//-----------------------------------------------------------------------------
//      キーコンフィグを取得します.
//-----------------------------------------------------------------------------
const ImGuiRingMenu::KeyConfig& ImGuiRingMenu::GetKeyConfig() const
{ return m_KeyConfig; }

//-----------------------------------------------------------------------------
//      アニメーションスピードを設定します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::SetAnimationSpeed(float value)
{ m_AnimSpeed = value; }

//-----------------------------------------------------------------------------
//      アニメーションスピードを取得します.
//-----------------------------------------------------------------------------
float ImGuiRingMenu::GetAnimationSpeed() const
{ return m_AnimSpeed; }

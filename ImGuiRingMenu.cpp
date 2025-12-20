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
void DrawRingMenuItem(ImDrawList* dl, const ImGuiRingMenu::MenuItem& item, float iconSize, const ImVec2& pos, float alpha, bool selected)
{
    auto halfSize = iconSize * 0.5f;
    uint8_t a = uint8_t(255 * alpha);

    // アイコンを描画.
    if (item.Icon != ImTextureID_Invalid)
    {
        dl->AddImage(item.Icon,
            ImVec2(pos.x - halfSize, pos.y - halfSize),
            ImVec2(pos.x + halfSize, pos.y + halfSize));
    }
    // 頭文字を描画.
    else
    {
        dl->AddRectFilled(
            ImVec2(pos.x - halfSize, pos.y - halfSize),
            ImVec2(pos.x + halfSize, pos.y + halfSize),
            (selected ? IM_COL32(255, 255, 255, a) : IM_COL32(0, 0, 0, a)),
            2.0f);
        const char capital[2] = { item.Label.c_str()[0], '\0'};
        auto textSize = ImGui::CalcTextSize(capital);
        auto textScale = (halfSize / dl->_Data->FontSize);
        dl->AddText(
            dl->_Data->Font,
            halfSize,
            ImVec2(pos.x - textSize.x * 0.5f * textScale, pos.y - textSize.y * 0.5f * textScale),
            (selected ? IM_COL32(0, 0, 0, a) : IM_COL32(255, 255, 255, a)),
            capital);
    }

    // ラベル
    auto textSize  = ImGui::CalcTextSize(item.Label.c_str());
    auto textScale = (iconSize * 0.25f / dl->_Data->FontSize);
        dl->AddText(
            dl->_Data->Font,
            halfSize * 0.5f,
            ImVec2(pos.x - textSize.x * 0.5f * textScale, pos.y + iconSize * 0.6f),
            (selected ? IM_COL32(255, 255, 0, a) : IM_COL32(255, 255, 255, a)),
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
        m_AnimProgress = ImClamp(m_AnimProgress + deltaSec * m_Config.AnimSpeed, 0.0f, 1.0f);
    }
    // 終了アニメ.
    else if (m_State == AnimOut)
    {
        m_AnimProgress = ImClamp(m_AnimProgress - deltaSec * m_Config.AnimSpeed, 0.0f, 1.0f);
        // 終了判定.
        if (m_AnimProgress <= 1e-6f)
        {
            m_State        = AnimNone;
            m_AnimProgress = 0.0f;
        }
    }

    // 回転角を補間.
    m_CurrentAngle = ImLerp(m_CurrentAngle, m_TargetAngle, deltaSec * m_Config.AnimSpeed);
 
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
    // いったん設定.
    selectedIndex = m_SelectedId;

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
    if (ImGui::IsKeyPressed(ImGuiKey(m_Config.KeyMenuStart)) && m_State == AnimNone)
    {
        m_State        = AnimIn;
        m_AnimProgress = 0.0f;
    }
    // メニュー終了.
    if (ImGui::IsKeyPressed(ImGuiKey(m_Config.KeyMenuEnd)) && m_State == AnimIn)
    {
        m_State        = AnimOut;
        m_AnimProgress = 1.0f;
    }
    // メニュー決定.
    if (ImGui::IsKeyPressed(ImGuiKey(m_Config.KeyConfirmation)) && m_State == AnimIn)
    {
        m_State        = AnimOut;
        m_AnimProgress = 1.0f;
        result = true;
    }

    // メニューが無効状態なら終了.
    if (m_State == AnimNone)
        return result;

    // 時計回りに回転.
    if (ImGui::IsKeyPressed(ImGuiKey(m_Config.KeyCwRotate)))
    {
        m_TargetAngle -= rotateStep;
        m_SelectedId++;
    }
    // 反時計周りに回転.
    if (ImGui::IsKeyPressed(ImGuiKey(m_Config.KeyCcwRotate)))
    {
        m_TargetAngle += rotateStep;
        m_SelectedId--;
    }

    // 番号をラップ.
    if (m_SelectedId >= count)
        m_SelectedId = 0;
    else if (m_SelectedId < 0)
        m_SelectedId = count - 1;

    // 補正済み値を設定.
    selectedIndex = m_SelectedId;

    auto r = ImLerp(radius * 3.0f, radius, m_AnimProgress);   // 描画半径.
    auto startAngle = -IM_PI * 0.5f + (1.0f - m_AnimProgress) * IM_PI;    // 開始角度.
    auto endAngle   =  IM_PI * 1.5f + (1.0f - m_AnimProgress) * IM_PI;    // 終了角度.

    // 背景の描画リストを取得.
    auto dl = ImGui::GetBackgroundDrawList();

    // 各メニュー項目を描画.
    for(auto i=0; i<count; ++i)
    {
        auto t     = float(i) / float(count);
        auto angle = startAngle + (endAngle - startAngle) * t + m_CurrentAngle;

        ImVec2 pos(center.x + cosf(angle) * r,
                   center.y + sinf(angle) * r);

        int index = i % count;
        DrawRingMenuItem(dl, m_Items[index], m_Config.IconSize, pos, m_AnimProgress, i == m_SelectedId);
    }

    auto halfSize = m_Config.IconSize * 0.5f;
    auto lineLen  = halfSize * 0.5f;
    auto lineCol  = IM_COL32(0, 0, 255, 255);
    auto thickness = 4.0f;

    // 枠描画

    // 左上.
    dl->AddLine(
        ImVec2(center.x - halfSize - thickness,          center.y - r - halfSize - thickness),
        ImVec2(center.x - halfSize - thickness+ lineLen, center.y - r - halfSize - thickness),
        lineCol,
        thickness);
    dl->AddLine(
        ImVec2(center.x - halfSize - thickness, center.y - r - halfSize - thickness),
        ImVec2(center.x - halfSize - thickness, center.y - r - halfSize - thickness + lineLen),
        lineCol,
        thickness);

    // 右上.
    dl->AddLine(
        ImVec2(center.x + halfSize + thickness - 1,           center.y - r - halfSize - thickness),
        ImVec2(center.x + halfSize + thickness - 1 - lineLen, center.y - r - halfSize - thickness),
        lineCol,
        thickness);
    dl->AddLine(
        ImVec2(center.x + halfSize + thickness - 1, center.y - r - halfSize - thickness),
        ImVec2(center.x + halfSize + thickness - 1, center.y - r - halfSize - thickness + lineLen),
        lineCol,
        thickness);

    // 左下.
    dl->AddLine(
        ImVec2(center.x - halfSize - thickness,             center.y - r + halfSize + thickness - 1),
        ImVec2(center.x - halfSize - thickness + lineLen,   center.y - r + halfSize + thickness - 1),
        lineCol,
        thickness);
    dl->AddLine(
        ImVec2(center.x - halfSize - thickness, center.y - r + halfSize + thickness - 1),
        ImVec2(center.x - halfSize - thickness, center.y - r + halfSize + thickness - 1 - lineLen),
        lineCol,
        thickness);

    // 右下.
    dl->AddLine(
        ImVec2(center.x + halfSize + thickness - 1,             center.y - r + halfSize + thickness - 1),
        ImVec2(center.x + halfSize + thickness - 1 - lineLen,   center.y - r + halfSize + thickness - 1),
        lineCol,
        thickness);
    dl->AddLine(
        ImVec2(center.x + halfSize + thickness - 1, center.y - r + halfSize + thickness - 1),
        ImVec2(center.x + halfSize + thickness - 1, center.y - r + halfSize + thickness - 1 - lineLen),
        lineCol,
        thickness);

    // 選択されたら true を返す.
    return result;
}

//-----------------------------------------------------------------------------
//      コンフィグを設定します.
//-----------------------------------------------------------------------------
void ImGuiRingMenu::SetConfig(const Config& value)
{ m_Config = value; }

//-----------------------------------------------------------------------------
//      コンフィグを取得します.
//-----------------------------------------------------------------------------
const ImGuiRingMenu::Config& ImGuiRingMenu::GetConfig() const
{ return m_Config; }

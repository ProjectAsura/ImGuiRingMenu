# ImGuiRingMenu

ImGuiを用いて聖剣伝説風のリングメニューを描画します.  



## 組み込み方法について
ImGuiRingMenu.cppとImGuiRingMenu.hをプロジェクトに追加してください。  

## 使用方法について
メニュー項目は次のように追加します。  

~~~~~
ImGuiRingMenu menu;
menu.Add({"Menu1", MenuIcon1});
menu.Add({"Menu2", MenuIcon2});
menu.Add({"Menu3", ImTexureID_Invalid});
~~~~~

毎フレームImGuiRingMenu::Update()と，ImGuiRingMenu::Draw()を呼び出してください.  


~~~~~
// アニメーションを更新.
menu.Update(deltaSeconds);

// 描画処理.
if (menu.Draw(selectedItemIndex))
{
    // ここで，選択された項目に応じた処理を行う.
}
~~~~~

## ライセンスについて
MIT License.


// Copyright 2019 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/combo_box.h"

#include "base/strings/utf_string_conversions.h"
#include "nativeui/win/picker_win.h"
#include "nativeui/win/util/hwnd_util.h"

namespace nu {

namespace {

class ComboBoxImpl : public PickerImpl {
 public:
  explicit ComboBoxImpl(ComboBox* delegate)
    : PickerImpl(delegate, CBS_DROPDOWN) {}

  // PickerImpl:
  void OnCommand(UINT code, int command) {
    ComboBox* combobox = static_cast<ComboBox*>(delegate());
    if (code == CBN_EDITCHANGE)
      combobox->on_text_change.Emit(combobox);
    PickerImpl::OnCommand(code, command);
  }
};

}  // namespace

ComboBox::ComboBox() : Picker(new ComboBoxImpl(this)) {
}

ComboBox::~ComboBox() {
}

void ComboBox::SetText(const std::string& text) {
  auto* combobox = static_cast<ComboBoxImpl*>(GetNative());
  base::string16 text16 = base::UTF8ToUTF16(text);
  ::SetWindowTextW(combobox->hwnd(), text16.c_str());
}

std::string ComboBox::GetText() const {
  auto* combobox = static_cast<ComboBoxImpl*>(GetNative());
  return base::UTF16ToUTF8(GetWindowString(combobox->hwnd()));
}

void ComboBox::AddItem(const std::string& text) {
  auto* picker = static_cast<PickerImpl*>(GetNative());
  base::string16 text16 = base::UTF8ToUTF16(text);
  ::SendMessage(picker->hwnd(), CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(text16.c_str()));
}

}  // namespace nu

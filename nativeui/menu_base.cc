// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/menu_base.h"

#include <algorithm>
#include <utility>

#include "nativeui/menu_item.h"

namespace nu {

MenuBase::MenuBase(NativeMenu menu) : menu_(menu) {
  PlatformInit();
}

MenuBase::~MenuBase() {
  PlatformDestroy();
}

void MenuBase::Append(scoped_refptr<MenuItem> item) {
  Insert(std::move(item), ItemCount());
}

void MenuBase::Insert(scoped_refptr<MenuItem> item, int index) {
  if (!item || item->GetMenu() || index < 0 || index > ItemCount())
    return;
  item->set_menu(this);
  item->SetAcceleratorManager(accel_manager_);
  PlatformInsert(item.get(), index);
  items_.insert(items_.begin() + index, std::move(item));
}

void MenuBase::Remove(MenuItem* item) {
  const auto i(std::find(items_.begin(), items_.end(), item));
  if (i == items_.end())
    return;
  item->set_menu(nullptr);
  item->SetAcceleratorManager(nullptr);
  PlatformRemove(item);
  items_.erase(i);
}

void MenuBase::SetAcceleratorManager(AcceleratorManager* accel_manager) {
  accel_manager_ = accel_manager;
  for (int i = 0; i < ItemCount(); ++i)
    ItemAt(i)->SetAcceleratorManager(accel_manager);
}

}  // namespace nu

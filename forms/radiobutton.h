#pragma once

#include "checkbox.h"
#include <list>

namespace OpenApoc
{

class RadioButton;

class RadioButtonGroup
{
  public:
	RadioButtonGroup(UString ID);
	UString ID;
	std::list<wp<RadioButton>> radioButtons;
};

class RadioButton : public CheckBox
{

  private:
	sp<RadioButtonGroup> group;

  public:
	RadioButton(sp<RadioButtonGroup> radioButtonGroup = nullptr, sp<Image> ImageChecked = nullptr,
	            sp<Image> ImageUnchecked = nullptr);
	virtual ~RadioButton();
	void SetChecked(bool checked) override;

	virtual sp<Control> CopyTo(sp<Control> CopyParent) override;
};

}; // namespace OpenApoc

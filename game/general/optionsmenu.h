
#pragma once

#include "framework/stage.h"

#include "forms/forms.h"

namespace OpenApoc
{

class OptionsMenu : public Stage
{
  private:
	sp<Form> menuform;
	StageCmd stageCmd;

  public:
	OptionsMenu();
	~OptionsMenu();
	// Stage control
	virtual void Begin() override;
	virtual void Pause() override;
	virtual void Resume() override;
	virtual void Finish() override;
	virtual void EventOccurred(Event *e) override;
	virtual void Update(StageCmd *const cmd) override;
	virtual void Render() override;
	virtual bool IsTransition() override;
};
}; // namespace OpenApoc

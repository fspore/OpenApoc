#include "library/sp.h"

#include "ufopaediacategory.h"
#include "ufopaedia.h"
#include "framework/framework.h"

namespace OpenApoc
{

UfopaediaCategory::UfopaediaCategory(tinyxml2::XMLElement *Element)
    : Stage(), menuform(fw().gamecore->GetForm("FORM_UFOPAEDIA_BASE"))
{
	UString nodename;

	ViewingEntry = 0;

	if (Element->Attribute("id") != nullptr && UString(Element->Attribute("id")) != "")
	{
		ID = Element->Attribute("id");
	}

	tinyxml2::XMLElement *node;
	for (node = Element->FirstChildElement(); node != nullptr; node = node->NextSiblingElement())
	{
		nodename = node->Name();

		if (nodename == "backgroundimage")
		{
			BackgroundImageFilename = node->GetText();
		}
		if (nodename == "title")
		{
			Title = node->GetText();
		}
		if (nodename == "text_info")
		{
			BodyInformation = node->GetText();
		}
		if (nodename == "entries")
		{
			tinyxml2::XMLElement *node2;
			for (node2 = node->FirstChildElement(); node2 != nullptr;
			     node2 = node2->NextSiblingElement())
			{
				sp<UfopaediaEntry> newentry = mksp<UfopaediaEntry>(node2);
				Entries.push_back(newentry);
			}
		}
	}
}

UfopaediaCategory::~UfopaediaCategory() {}

void UfopaediaCategory::Begin()
{
	auto infolabel = menuform->FindControlTyped<Label>("TEXT_INFO");
	auto entrylist = menuform->FindControlTyped<ListBox>("LISTBOX_SHORTCUTS");
	entrylist->Clear();
	entrylist->ItemSize = infolabel->GetFont()->GetFontHeight() + 2;
	int idx = 1;
	for (auto entry = Entries.begin(); entry != Entries.end(); entry++)
	{
		sp<UfopaediaEntry> e = *entry;
		auto tb = mksp<TextButton>(tr(e->Title, "paedia_string"), infolabel->GetFont());
		tb->Name = "Index" + Strings::FromInteger(idx);
		tb->RenderStyle = TextButton::TextButtonRenderStyles::SolidButtonStyle;
		tb->TextHAlign = HorizontalAlignment::Left;
		tb->TextVAlign = VerticalAlignment::Centre;
		entrylist->AddItem(tb);
		idx++;
	}

	SetupForm();
	SetTopic(0);
}

void UfopaediaCategory::Pause() {}

void UfopaediaCategory::Resume() {}

void UfopaediaCategory::Finish()
{
	// ListBox* entrylist = ((ListBox*)menuform->FindControl("LISTBOX_SHORTCUTS"));
	// entrylist->Clear();
}

void UfopaediaCategory::EventOccurred(Event *e)
{
	menuform->EventOccured(e);
	fw().gamecore->MouseCursor->EventOccured(e);

	if (e->Type() == EVENT_KEY_DOWN)
	{
		if (e->Keyboard().KeyCode == SDLK_ESCAPE)
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
	}

	if (e->Type() == EVENT_FORM_INTERACTION && e->Forms().EventFlag == FormEventType::ButtonClick)
	{
		if (e->Forms().RaisedBy->Name == "BUTTON_QUIT")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
		else if (e->Forms().RaisedBy->Name == "BUTTON_INFORMATION")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible =
			    !menuform->FindControl("INFORMATION_PANEL")->Visible;
		}
		else if (e->Forms().RaisedBy->Name == "BUTTON_NEXT_SECTION")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			SetNextCat();
			return;
		}
		else if (e->Forms().RaisedBy->Name == "BUTTON_NEXT_TOPIC")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			if (ViewingEntry < Entries.size())
			{
				SetTopic(ViewingEntry + 1);
			}
			else
			{
				SetNextCat();
			}
			return;
		}
		else if (e->Forms().RaisedBy->Name == "BUTTON_PREVIOUS_TOPIC")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			if (ViewingEntry > 0)
			{
				SetTopic(ViewingEntry - 1);
			}
			else
			{
				SetPrevCat();
			}
			return;
		}
		else if (e->Forms().RaisedBy->Name == "BUTTON_PREVIOUS_SECTION")
		{
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			SetPrevCat();
			return;
		}
		else if (e->Forms().RaisedBy->Name.substr(0, 5) == "Index")
		{
			UString nameidx =
			    e->Forms().RaisedBy->Name.substr(5, e->Forms().RaisedBy->Name.length() - 5);
			int idx = Strings::ToInteger(nameidx);
			SetTopic(idx);
			menuform->FindControl("INFORMATION_PANEL")->Visible = false;
			return;
		}
		else
		{
			return;
		}
	}
}

void UfopaediaCategory::Update(StageCmd *const cmd)
{
	menuform->Update();
	*cmd = this->stageCmd;
	// Reset the command to default
	this->stageCmd = StageCmd();
}

void UfopaediaCategory::Render()
{
	fw().Stage_GetPrevious(this->shared_from_this())->Render();
	// fw().renderer->drawFilledRect({0, 0}, fw().Display_GetSize(), Colour{0, 0, 0, 128});
	menuform->Render();
	fw().gamecore->MouseCursor->Render();
}

bool UfopaediaCategory::IsTransition() { return false; }

void UfopaediaCategory::SetTopic(int Index)
{
	ViewingEntry = Index;
	SetupForm();
}

void UfopaediaCategory::SetupForm()
{
	if (ViewingEntry == 0)
	{
		menuform->FindControlTyped<Graphic>("BACKGROUND_PICTURE")
		    ->SetImage(fw().data->load_image(BackgroundImageFilename));
		auto infolabel = menuform->FindControlTyped<Label>("TEXT_INFO");
		infolabel->SetText(tr(BodyInformation, "paedia_string"));
		infolabel = menuform->FindControlTyped<Label>("TEXT_TITLE_DATA");
		infolabel->SetText(tr(Title, "paedia_string").toUpper());
	}
	else
	{
		sp<UfopaediaEntry> e = Entries.at(ViewingEntry - 1);
		menuform->FindControlTyped<Graphic>("BACKGROUND_PICTURE")
		    ->SetImage(fw().data->load_image(e->BackgroundImageFilename));
		auto infolabel = menuform->FindControlTyped<Label>("TEXT_INFO");
		infolabel->SetText(tr(e->BodyInformation, "paedia_string"));
		infolabel = menuform->FindControlTyped<Label>("TEXT_TITLE_DATA");
		infolabel->SetText(tr(e->Title, "paedia_string").toUpper());
	}
}

void UfopaediaCategory::SetPrevCat() { SetCatOffset(-1); }

void UfopaediaCategory::SetNextCat() { SetCatOffset(1); }

void UfopaediaCategory::SetCatOffset(int Direction)
{
	for (size_t idx = (Direction > 0 ? 0 : 1);
	     idx < Ufopaedia::UfopaediaDB.size() - (Direction < 0 ? 0 : 1); idx++)
	{
		if (Ufopaedia::UfopaediaDB.at(idx).get() == this)
		{
			sp<UfopaediaCategory> nxt = Ufopaedia::UfopaediaDB.at(idx + Direction);
			stageCmd.cmd = StageCmd::Command::REPLACE;
			stageCmd.nextStage = nxt;
		}
	}
}

}; // namespace OpenApoc

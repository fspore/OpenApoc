#include "library/sp.h"

#include "forms/textedit.h"
#include "framework/framework.h"
#include "game/resources/gamecore.h"

namespace OpenApoc
{

TextEdit::TextEdit(UString Text, sp<BitmapFont> font)
    : Control(), caretDraw(false), caretTimer(0), text(Text), font(font), editting(false),
      editShift(false), editAltGr(false), SelectionStart(Text.length()),
      TextHAlign(HorizontalAlignment::Left), TextVAlign(VerticalAlignment::Centre)
{
	if (font)
	{
		palette = font->getPalette();
	}
}

TextEdit::~TextEdit() {}

void TextEdit::EventOccured(Event *e)
{
	UString keyname;

	Control::EventOccured(e);

	if (e->Type() == EVENT_FORM_INTERACTION)
	{
		if (e->Forms().RaisedBy == shared_from_this())
		{
			if (e->Forms().EventFlag == FormEventType::GotFocus ||
			    e->Forms().EventFlag == FormEventType::MouseClick ||
			    e->Forms().EventFlag == FormEventType::KeyDown)
			{
				editting = true;
				// e->Handled = true;
			}
			if (e->Forms().EventFlag == FormEventType::LostFocus)
			{
				editting = false;
				RaiseEvent(FormEventType::TextEditFinish);
				// e->Handled = true;
			}
		}
		else if (e->Forms().EventFlag == FormEventType::MouseClick)
		{
			editting = false;
			RaiseEvent(FormEventType::TextEditFinish);
		}

		if (e->Forms().EventFlag == FormEventType::KeyPress && editting)
		{
			switch (e->Forms().KeyInfo.KeyCode) // TODO: Check scancodes instead of keycodes?
			{
				case SDLK_BACKSPACE:
					if (SelectionStart > 0)
					{
						text.remove(SelectionStart - 1, 1);
						SelectionStart--;
						RaiseEvent(FormEventType::TextChanged);
					}
					e->Handled = true;
					break;
				case SDLK_DELETE:
					if (SelectionStart < text.length())
					{
						text.remove(SelectionStart, 1);
						RaiseEvent(FormEventType::TextChanged);
					}
					e->Handled = true;
					break;
				case SDLK_LEFT:
					if (SelectionStart > 0)
					{
						SelectionStart--;
					}
					e->Handled = true;
					break;
				case SDLK_RIGHT:
					if (SelectionStart < text.length())
					{
						SelectionStart++;
					}
					e->Handled = true;
					break;
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					editShift = true;
					break;
				case SDLK_RALT:
					editAltGr = true;
					break;

				case SDLK_HOME:
					SelectionStart = 0;
					e->Handled = true;
					break;
				case SDLK_END:
					SelectionStart = text.length();
					e->Handled = true;
					break;

				case SDLK_RETURN:
					editting = false;
					RaiseEvent(FormEventType::TextEditFinish);
					break;

				default:
					// FIXME: This should use SDL Text Input API!
					UString convert(SDL_GetKeyName(
					    e->Keyboard()
					        .KeyCode)); // SDLK* are based on Unicode, if I read the docs right
					if (convert.length() == 1 && convert.c_str()[0] != 0)
					{
						text.insert(SelectionStart, convert.c_str());
						SelectionStart++;
						RaiseEvent(FormEventType::TextChanged);
					}
			}
		}

		if (e->Forms().EventFlag == FormEventType::KeyUp && editting)
		{

			switch (e->Forms().KeyInfo.KeyCode)
			{
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					editShift = false;
					e->Handled = true;
					break;
				case SDLK_RALT:
					editAltGr = false;
					e->Handled = true;
					break;
			}
		}
	}
}

void TextEdit::OnRender()
{
	int xpos;
	int ypos;

	switch (TextHAlign)
	{
		case HorizontalAlignment::Left:
			xpos = 0;
			break;
		case HorizontalAlignment::Centre:
			xpos = (Size.x / 2) - (font->GetFontWidth(text) / 2);
			break;
		case HorizontalAlignment::Right:
			xpos = Size.x - font->GetFontWidth(text);
			break;
		default:
			LogError("Unknown TextHAlign");
			return;
	}

	switch (TextVAlign)
	{
		case VerticalAlignment::Top:
			ypos = 0;
			break;
		case VerticalAlignment::Centre:
			ypos = (Size.y / 2) - (font->GetFontHeight() / 2);
			break;
		case VerticalAlignment::Bottom:
			ypos = Size.y - font->GetFontHeight();
			break;
		default:
			LogError("Unknown TextVAlign");
			return;
	}

	if (editting)
	{
		int cxpos = xpos + font->GetFontWidth(text.substr(0, SelectionStart)) + 1;

		if (cxpos < 0)
		{
			xpos += cxpos;
			cxpos = xpos + font->GetFontWidth(text.substr(0, SelectionStart)) + 1;
		}
		if (cxpos > Size.x)
		{
			xpos -= cxpos - Size.x;
			cxpos = xpos + font->GetFontWidth(text.substr(0, SelectionStart)) + 1;
		}

		if (caretDraw)
		{
			fw().renderer->drawLine(Vec2<float>{cxpos, ypos},
			                        Vec2<float>{cxpos, ypos + font->GetFontHeight()},
			                        Colour{255, 255, 255});
		}
	}

	auto textImage = font->getString(text);
	fw().renderer->draw(textImage, Vec2<float>{xpos, ypos});
}

void TextEdit::Update()
{
	if (editting)
	{
		caretTimer = (caretTimer + 1) % TEXTEDITOR_CARET_TOGGLE_TIME;
		if (caretTimer == 0)
		{
			caretDraw = !caretDraw;
		}
	}
}

void TextEdit::UnloadResources() {}

UString TextEdit::GetText() const { return text; }

void TextEdit::SetText(UString Text)
{
	text = Text;
	SelectionStart = text.length();
	RaiseEvent(FormEventType::TextChanged);
}

void TextEdit::RaiseEvent(FormEventType Type)
{
	std::ignore = Type;
	this->pushFormEvent(FormEventType::TextChanged, nullptr);
}

sp<BitmapFont> TextEdit::GetFont() const { return font; }

void TextEdit::SetFont(sp<BitmapFont> NewFont) { font = NewFont; }

sp<Control> TextEdit::CopyTo(sp<Control> CopyParent)
{
	sp<TextEdit> copy;
	if (CopyParent)
	{
		copy = CopyParent->createChild<TextEdit>(this->text, this->font);
	}
	else
	{
		copy = mksp<TextEdit>(this->text, this->font);
	}
	copy->TextHAlign = this->TextHAlign;
	copy->TextVAlign = this->TextVAlign;
	CopyControlData(copy);
	return copy;
}

void TextEdit::ConfigureFromXML(tinyxml2::XMLElement *Element)
{
	Control::ConfigureFromXML(Element);
	tinyxml2::XMLElement *subnode;
	UString attribvalue;

	if (Element->Attribute("text") != nullptr)
	{
		text = tr(Element->Attribute("text"));
	}
	if (Element->FirstChildElement("font") != nullptr)
	{
		font = fw().gamecore->GetFont(Element->FirstChildElement("font")->GetText());
	}
	subnode = Element->FirstChildElement("alignment");
	if (subnode != nullptr)
	{
		if (subnode->Attribute("horizontal") != nullptr)
		{
			attribvalue = subnode->Attribute("horizontal");
			if (attribvalue == "left")
			{
				TextHAlign = HorizontalAlignment::Left;
			}
			else if (attribvalue == "centre")
			{
				TextHAlign = HorizontalAlignment::Centre;
			}
			else if (attribvalue == "right")
			{
				TextHAlign = HorizontalAlignment::Right;
			}
		}
		if (subnode->Attribute("vertical") != nullptr)
		{
			attribvalue = subnode->Attribute("vertical");
			if (attribvalue == "top")
			{
				TextVAlign = VerticalAlignment::Top;
			}
			else if (attribvalue == "centre")
			{
				TextVAlign = VerticalAlignment::Centre;
			}
			else if (attribvalue == "bottom")
			{
				TextVAlign = VerticalAlignment::Bottom;
			}
		}
	}
}
}; // namespace OpenApoc

#include "UI.h"
#include "Engine.h"
#include "Util.h"

const float BAR_WIDTH = 200.0f;

const Vector4 TEXT_COLOR = Vector4(1, 1, 1, 0.6f);
const Vector4 TEXT_COLOR_HOVER = Vector4(1,1,1,1);

const Vector4 BG_COLOR = Vector4(0.1f, 0.1f, 0.1f, 0.9f);
const float BG_COLOR_ALPHA_STEP = 0.2f;

const float FONT_HEIGHT = 20.0f;

const float PADDING_VERTICAL = 5.0f;
const float PADDING_HORIZONTAL = 15.0f;

const float START_PADDING_VERTICAL = 20.0f;

const float COLOR_PICKER_CELL_SIZE = 9.0f;
const uint32_t COLOR_PICKER_CELL_COUNT = 20;

const float SCROLLBAR_WIDTH = 20.0f;

const float CHECKBOX_SIZE = FONT_HEIGHT;
const float CHECKBOX_BORDER = 2.0f;

const float TITLE_BUTTON_WIDTH = 360.0f;
const float TITLE_BUTTON_HEIGHT = 50.0f;

const float TITLE_BUTTON_TEXT_HEIGHT = 35.0f;

#define STRING_TO_CHAR(a) &a[0u]

UIContext UI::GetUIContext(RenderEngine *engine)
{
	UIContext result = {};
	result.engine = engine;
	return result;
}

void UI::HandleMessage(UIContext *context, Message message, void *data)
{
	switch (message)
	{
	case MOUSE_CHANGE:
	{
		MOUSE_CHANGE_DATA *mouseData = (MOUSE_CHANGE_DATA *)data;
		context->mousePosition = Vector2(mouseData->positionX,
										mouseData->positionY);
		context->mousePositionDelta = Vector2(mouseData->dx, mouseData->dy);
		context->mouseDown = mouseData->leftButtonDown;
		context->mousePressed = mouseData->leftButtonPressed;
		context->mouseScroll = mouseData->scrollDelta;
	} break;
	}
}

void UI::NewWindow(UIContext *context, Vector2 offset)
{
	context->mousePressed = false;
	context->mousePositionDelta = Vector2(0, 0);
	context->barID = 0;
	context->mouseScroll = 0.0f;
	context->layoutPosition = Vector2(0, START_PADDING_VERTICAL) + offset;
	context->uiOffset = offset;
	context->backgroundColor = BG_COLOR;
	REngine::RenderRectangle(context->engine, Vector2(context->layoutPosition.x, offset.y), Vector2(BAR_WIDTH, context->engine->screenSize.y - offset.y), context->backgroundColor);
}


void UI::NextBar(UIContext *context)
{
	++context->barID;
	context->layoutPosition.x = BAR_WIDTH * context->barID;
	context->layoutPosition.y = START_PADDING_VERTICAL + context->uiOffset.y;
	context->backgroundColor.w -= BG_COLOR_ALPHA_STEP;
	REngine::RenderRectangle(context->engine, Vector2(context->layoutPosition.x, context->uiOffset.y), Vector2(BAR_WIDTH, context->engine->screenSize.y - context->uiOffset.y), context->backgroundColor);
}

uint64_t HashString(char *string)
{
	uint64_t hash = 5381;
	int c;
	while (c = *string++)
		hash = ((hash << 5) + hash) ^ c; /* hash * 33 xor c */

	return hash + 1;
}

void SetActive(UIContext *context, uint64_t id)
{
	context->activeID = id;
	Messaging::Dispatch(context->messenger, UI_INPUT_LOCK, NULL);
}

void UnPress(UIContext *context)
{
	context->activeID = 0;
	Messaging::Dispatch(context->messenger, UI_INPUT_UNLOCK, NULL);
}

bool IsActive(UIContext *context, uint64_t id)
{
	return context->activeID == id;
}

bool IsHovered(UIContext *context, Rect rect)
{
	if (context->mousePosition.x > rect.left && context->mousePosition.x < rect.left + rect.width &&
		context->mousePosition.y > rect.top && context->mousePosition.y < rect.top + rect.height)
		return true;
	return false;
}

Vector4 UI::ColorPicker(UIContext *context, char *label, Vector4 color)
{
	RenderEngine *renderEngine = context->engine;
	Vector4 hsvColor = Util::RGBtoHSV(color);
	uint64_t id = HashString(label);

	context->layoutPosition.y += PADDING_VERTICAL;
	REngine::RenderText(renderEngine, label, context->layoutPosition + Vector2(BAR_WIDTH / 2.0f, 0.0f), TEXT_COLOR, FONT_HEIGHT, Vector2(0.5f, 0.0f));
	context->layoutPosition.y += PADDING_VERTICAL + FONT_HEIGHT;

	Vector2 position = context->layoutPosition;
	position.x += (BAR_WIDTH - COLOR_PICKER_CELL_COUNT * COLOR_PICKER_CELL_SIZE) / 2.0f;
	for (uint32_t hi = 0; hi < COLOR_PICKER_CELL_COUNT; ++hi)
	{
		position.y = context->layoutPosition.y;
		for (uint32_t si = 0; si < COLOR_PICKER_CELL_COUNT; ++si)
		{
			float h = hi / (float)COLOR_PICKER_CELL_COUNT;
			float s = si / (float)COLOR_PICKER_CELL_COUNT;
			float v = hsvColor.z;
			Vector4 rgbColor = Util::HSVtoRGB(Vector4(h, s, v, 1.0f));
			Rect pickerFieldRect = Rect(position.x, position.y, COLOR_PICKER_CELL_SIZE, COLOR_PICKER_CELL_SIZE);
			REngine::RenderRectangle(renderEngine, position,
									 Vector2(pickerFieldRect.width, pickerFieldRect.height),
									 rgbColor);
			if (IsHovered(context, pickerFieldRect))
			{
				if (!IsActive(context, id) && context->mousePressed)
				{
					SetActive(context, id);
				}
				if (IsActive(context, id))
				{
					hsvColor = Vector4(h, s, v, hsvColor.w);
				}
			}
			position.y += COLOR_PICKER_CELL_SIZE;
		}
		position.x += COLOR_PICKER_CELL_SIZE;
	}

	context->layoutPosition.y += COLOR_PICKER_CELL_COUNT * COLOR_PICKER_CELL_SIZE + 2 * PADDING_VERTICAL;

	position = context->layoutPosition;
	position.x += (BAR_WIDTH - COLOR_PICKER_CELL_COUNT * COLOR_PICKER_CELL_SIZE) / 2.0f;
	for (uint32_t vi = 0; vi < COLOR_PICKER_CELL_COUNT; ++vi)
	{
		float h = hsvColor.x;
		float s = hsvColor.y;
		float v = vi / (float)COLOR_PICKER_CELL_COUNT;

		Vector4 rgbColor = Util::HSVtoRGB(Vector4(h, s, v, 1.0f));
		Rect pickerFieldRect = Rect(position.x, position.y, COLOR_PICKER_CELL_SIZE, COLOR_PICKER_CELL_SIZE);
		REngine::RenderRectangle(renderEngine, position, 
								 Vector2(pickerFieldRect.width, pickerFieldRect.height), 
								 rgbColor);
		if (IsHovered(context, pickerFieldRect))
		{
			if (!IsActive(context, id) && context->mousePressed)
			{
				SetActive(context, id);
			}
			if (IsActive(context, id))
			{
				hsvColor = Vector4(h, s, v, hsvColor.w);
			}
		}
		position.x += COLOR_PICKER_CELL_SIZE;
	}
	if (IsActive(context, id) && !context->mouseDown)
	{
		UnPress(context);
	}
	context->layoutPosition.y += COLOR_PICKER_CELL_SIZE + PADDING_VERTICAL;
	return Util::HSVtoRGB(hsvColor);
}

int32_t UI::Menu(UIContext *context, std::string *labels, uint32_t labelCount, int32_t pressed)
{
	RenderEngine *engine = context->engine;
	int32_t currentState = - 1;
	
	for (int32_t i = 0; i < labelCount; i++)
	{
		if (UI::Button(context, STRING_TO_CHAR(labels[i]), i == pressed)) 
		{
			currentState = i;
			//if (currentState != pressed)
			//	break;
		}
		
		
	}
	return currentState;
}

bool UI::Button(UIContext *context, char *label, bool pressed, bool checkBox)
{
	RenderEngine *engine = context->engine;

	uint64_t id = HashString(label);
	Rect buttonRect = Rect(context->layoutPosition.x, context->layoutPosition.y, BAR_WIDTH, FONT_HEIGHT + 2 * PADDING_VERTICAL);
	Vector4 color = TEXT_COLOR;
	if (IsHovered(context, buttonRect))
	{
		if (context->mousePressed)
		{
			pressed = !pressed;
			UnPress(context);
		}
	}
	if (IsHovered(context, buttonRect) || (pressed && !checkBox))
	{
		color = TEXT_COLOR_HOVER;
	}

	float textFieldWidth = checkBox ? (BAR_WIDTH - 2 * PADDING_HORIZONTAL) : BAR_WIDTH;
	context->layoutPosition.y += PADDING_VERTICAL;
	REngine::RenderText(engine, label, context->layoutPosition + Vector2(textFieldWidth / 2.0f, 0), color, FONT_HEIGHT, Vector2(0.5f, 0));
	if (checkBox)
	{
		Vector2 checkBoxPosition = Vector2(context->layoutPosition.x + textFieldWidth, context->layoutPosition.y);
		REngine::RenderRectangle(engine, checkBoxPosition, Vector2(CHECKBOX_SIZE, CHECKBOX_SIZE), TEXT_COLOR);
		if (!pressed)
		{
			float innerCheckboxSize = CHECKBOX_SIZE - CHECKBOX_BORDER * 2.0f;
			REngine::RenderRectangle(engine, checkBoxPosition + Vector2(CHECKBOX_BORDER, CHECKBOX_BORDER), Vector2(innerCheckboxSize, innerCheckboxSize), context->backgroundColor);
		}
	}
	context->layoutPosition.y += FONT_HEIGHT + PADDING_VERTICAL;
	return pressed;
}

bool UI::Button(UIContext *context, char *text, bool pressed, Vector2 position)
{
	RenderEngine *renderEngine = context->engine;
	uint64_t id = HashString(text);
	Rect buttonRect = Rect(position.x - TITLE_BUTTON_WIDTH / 2.0f, position.y - TITLE_BUTTON_HEIGHT / 2.0f, TITLE_BUTTON_WIDTH, TITLE_BUTTON_HEIGHT);
	Vector4 textColor = TEXT_COLOR;
	if (IsHovered(context, buttonRect))
	{
		textColor = TEXT_COLOR_HOVER;
		if (context->mousePressed)
		{
			pressed = !pressed;
		}
	}
	REngine::RenderRectangle(renderEngine, position + Vector2(-TITLE_BUTTON_WIDTH / 2.0f, -TITLE_BUTTON_HEIGHT / 2.0f), Vector2(TITLE_BUTTON_WIDTH, TITLE_BUTTON_HEIGHT), BG_COLOR);
	REngine::RenderText(renderEngine, "NEW TREE", position, textColor, TITLE_BUTTON_TEXT_HEIGHT, Vector2(0.5f, 0.5f));
	return pressed;
}


uint32_t UI::DropDown(UIContext *context, char *label, char **options, uint32_t optionCount, uint32_t selectedOption, uint32_t numberOfShownOptions)
{
	uint64_t id = HashString(label);
	Vector4 color = TEXT_COLOR;
	Vector2 position = context->layoutPosition;
	position.x += BAR_WIDTH / 2.0f;

	REngine::RenderText(context->engine, label, position, color, FONT_HEIGHT, Vector2(0.5f, 0));

	position.x -= SCROLLBAR_WIDTH / 2.0f;
	position.y += PADDING_VERTICAL + FONT_HEIGHT;
	Rect mainPart = Rect(context->layoutPosition.x, position.y, BAR_WIDTH, FONT_HEIGHT + 2 * PADDING_VERTICAL);
	if (IsHovered(context, mainPart))
	{
		if (!IsActive(context, id) && context->mousePressed)
		{
			SetActive(context, id);
			context->scrollbarPosition = 0.0f;
		} 
		else if (IsActive(context, id) && context->mousePressed)
		{
			UnPress(context);
		}
	}
	if (IsHovered(context, mainPart) || IsActive(context, id))
	{
		color = TEXT_COLOR_HOVER;
	}

	REngine::RenderText(context->engine, options[selectedOption], position, color, FONT_HEIGHT, Vector2(0.5f, 0));
	position.y += FONT_HEIGHT + PADDING_VERTICAL;

	if (IsActive(context, id))
	{
		float scrollSectionHeight = (FONT_HEIGHT + 2 * PADDING_VERTICAL) * numberOfShownOptions - PADDING_VERTICAL * 2.0f;
		mainPart.height += scrollSectionHeight + 2 * PADDING_VERTICAL;
		if (IsHovered(context, mainPart))
		{
			context->scrollbarPosition -= context->mouseScroll / 120.0f;
		}
		context->scrollbarPosition = MAX(0, context->scrollbarPosition);
		context->scrollbarPosition = MIN(context->scrollbarPosition, optionCount - numberOfShownOptions);
		float part = context->scrollbarPosition / (float)(optionCount - numberOfShownOptions);
		position.y += PADDING_VERTICAL;

		REngine::RenderRectangle(context->engine, 
								 Vector2(context->layoutPosition.x + PADDING_HORIZONTAL, position.y - 4), 
								 Vector2(BAR_WIDTH - 2 * PADDING_HORIZONTAL, 1.5), 
								 TEXT_COLOR);
		REngine::RenderRectangle(context->engine, 
								 Vector2(context->layoutPosition.x + BAR_WIDTH - PADDING_HORIZONTAL - SCROLLBAR_WIDTH, 
										 position.y + PADDING_VERTICAL), 
								 Vector2(SCROLLBAR_WIDTH, scrollSectionHeight),  
								 TEXT_COLOR * 0.5f);
		REngine::RenderRectangle(context->engine, 
								 Vector2(context->layoutPosition.x + BAR_WIDTH - PADDING_HORIZONTAL - SCROLLBAR_WIDTH, 
										 position.y + PADDING_VERTICAL + part * (scrollSectionHeight - (FONT_HEIGHT + PADDING_VERTICAL))),
								 Vector2(SCROLLBAR_WIDTH, PADDING_VERTICAL + FONT_HEIGHT), 
								 TEXT_COLOR);
		Rect scrollBarRect = Rect(context->layoutPosition.x + BAR_WIDTH - PADDING_HORIZONTAL - SCROLLBAR_WIDTH, position.y + PADDING_VERTICAL,
								  SCROLLBAR_WIDTH, scrollSectionHeight);
		Rect scrollBarControllerRect = Rect(context->layoutPosition.x + BAR_WIDTH - PADDING_HORIZONTAL - SCROLLBAR_WIDTH, position.y + PADDING_VERTICAL + part * (scrollSectionHeight - (FONT_HEIGHT + PADDING_VERTICAL)),
								  SCROLLBAR_WIDTH, PADDING_VERTICAL + FONT_HEIGHT);

		uint32_t startingIndex = (uint32_t) context->scrollbarPosition;
		uint32_t endingIndex = MIN((uint32_t)context->scrollbarPosition + numberOfShownOptions, optionCount);
		if(IsHovered(context, scrollBarControllerRect))
		{ 
			if (context->mousePressed)
			{
				context->scrollbarActive = true;
			}
		}
		else if (IsHovered(context, scrollBarRect))
		{
			if (context->mousePressed)
			{
				context->scrollbarPosition = (context->mousePosition.y - position.y - PADDING_VERTICAL) / scrollSectionHeight * (optionCount - numberOfShownOptions);
			}
		}
		if (!IsHovered(context, scrollBarControllerRect))
		{
			if (!context->mouseDown)
			{
				context->scrollbarActive = false;
			}
		}
		if (context->scrollbarActive)
		{
			context->scrollbarPosition += context->mousePositionDelta.y / (scrollSectionHeight - (PADDING_VERTICAL + FONT_HEIGHT)) * (optionCount - numberOfShownOptions);
		}
		for (uint32_t i = startingIndex; i < endingIndex; ++i)
		{
			Vector4 color = TEXT_COLOR;
			Rect currentPart = Rect(context->layoutPosition.x, position.y, BAR_WIDTH - PADDING_HORIZONTAL - SCROLLBAR_WIDTH, FONT_HEIGHT + 2 * PADDING_VERTICAL);
			position.y += PADDING_VERTICAL;
			if (IsHovered(context, currentPart))
			{
				if (context->mousePressed)
				{
					selectedOption = i;
				}
				color = TEXT_COLOR_HOVER;
			}
			char *option = options[i];
			REngine::RenderText(context->engine, option, position, color, FONT_HEIGHT, Vector2(0.5f, 0));
			position.y += FONT_HEIGHT + PADDING_VERTICAL;
		}
	}
	if (IsActive(context, id) && !IsHovered(context, mainPart) && context->mousePressed)
	{
		UnPress(context);
	}
	context->layoutPosition.y = position.y;
	return selectedOption;
}

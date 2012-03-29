#include "StdAfx.h"
#include "ForcedDeclaration.h"
#include "Render.h"
#include "Universe.h"
#include "EditorEventReceiver.h"

// ��� ����������� ���������� �������
bool EditorEventReceiver::OnEvent(const SEvent& event)
{
	// ������ ���������� ��������� ����� ������� - ������/������
	if (event.EventType == irr::EET_KEY_INPUT_EVENT)
	{
		KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
		if (KeyIsDown[KEY_ESCAPE])
		{
			delete Universe::instance->gameName;
			Universe::instance->gameName = NULL;
			Universe::instance->state = NextLevel;
		}
	}
	else if (event.EventType == EET_GUI_EVENT)
	{
		s32 id = event.GUIEvent.Caller->getID(); // �������� ������������� ��������, ���������� �������

		switch(event.GUIEvent.EventType) // ������������ ������� ������������ ����
		{
			case EGET_BUTTON_CLICKED:
				if (id >= MapCellSelectWindowToggleButton && id <= CharacterSelectWindowToggleButton)
				{
					if (!Universe::instance->menuGuienv->getRootGUIElement()->getElementFromId(id - 10))
					{
						IGUIWindow* wnd1;
						wnd1 = Universe::instance->guienv->addWindow(rect< s32 >(224, 64, 696, 536), false, L"MapObject select", 0, id - 10);
						//Universe::instance->menuGuienv->getRootGUIElement()->getElementFromId(id - 10)->setVisible(true);
					}
					Universe::instance->guienv->setFocus(Universe::instance->menuGuienv->getRootGUIElement()->getElementFromId(id - 10));
				}
				switch (id)
				{
					case MapCellSelectWindowToggleButton:
						break;
					case NPCSelectWindowToggleButton:
						break;
					case ItemSelectWindowToggleButton:
						break;
					case StaticSelectWindowToggleButton:
						break;
					case CharacterSelectWindowToggleButton:
						break;
				}
				break;
			case EGET_COMBO_BOX_CHANGED:
				switch (id)
				{
					case LocationsComboBox:
						Universe::instance->SetLocation(Universe::instance->game->data->locations[Universe::instance->locationsComboBox->getSelected()]);
						break;
				}
				break;
		}
	}

	return false;
}

// ����� ������������ ��������� ��� ����������� �������
bool EditorEventReceiver::IsKeyDown(EKEY_CODE keyCode) const
{
	return KeyIsDown[keyCode];
}
    
//�����������, � ����� ���������� ������ ��� ���� ������
EditorEventReceiver::EditorEventReceiver()
{
	for (u32 i = 0; i < KEY_KEY_CODES_COUNT; ++i)
		KeyIsDown[i] = false;
}

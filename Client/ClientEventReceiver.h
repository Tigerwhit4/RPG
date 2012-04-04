#pragma once

class ClientEventReceiver :
	public IEventReceiver
{
public:
	// ��� ����������� ���������� �������
	virtual bool OnEvent(const SEvent& event);
	// ����� ������������ ��������� ��� ����������� �������
	virtual bool IsKeyDown(EKEY_CODE keyCode) const;
	//�����������, � ����� ���������� ������ ��� ���� ������
	ClientEventReceiver();
private:
	// ������ ��� �������� �������� ������
	bool KeyIsDown[KEY_KEY_CODES_COUNT];
};

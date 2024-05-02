#pragma once

class CIndexCreator
{
	DWORD*			m_pdwIndexTable = nullptr;
	DWORD			m_dwMaxNum = 0;
	DWORD			m_dwAllocatedCount = 0;


public:

	BOOL			Initialize(DWORD dwNum);

	DWORD			Alloc();
	void			Free(DWORD dwIndex);
	void			Cleanup();
	void			Check();

	CIndexCreator();
	~CIndexCreator();
};

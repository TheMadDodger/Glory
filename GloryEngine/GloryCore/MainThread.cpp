#include "MainThread.h"

namespace Glory
{
	MainThread::MainThread()
	{
	}

	MainThread::~MainThread()
	{
	}

	void MainThread::Update()
	{
		for (size_t i = 0; i < m_MainUpdateBinds.size(); i++)
		{
			m_MainUpdateBinds[i]();
		}
	}
}
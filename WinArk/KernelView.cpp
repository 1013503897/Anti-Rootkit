#include "stdafx.h"
#include "resource.h"
#include "KernelView.h"
#include "ThemeSystem.h"

LRESULT CKernelView::OnCreate(UINT, WPARAM, LPARAM, BOOL&) {
	CRect r(0, 0, 400, 25);
	CTabCtrl tabCtrl;
	auto hTabCtrl = tabCtrl.Create(m_hWnd, &r, nullptr, WS_CHILDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS
		| TCS_HOTTRACK | TCS_SINGLELINE | TCS_RIGHTJUSTIFY | TCS_TABS,
		WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR | WS_EX_NOPARENTNOTIFY, TabId);
	m_TabCtrl.SubclassWindow(hTabCtrl);

	//m_TabCtrl.SetFont()

	HFONT hFont = g_hAppFont;
	m_TabCtrl.SetFont(hFont, true);

	m_KernelPoolView = new CKernelPoolView(m_pFrame);
	m_hwndArray[static_cast<int>(TabColumn::KernelPoolTable)] = m_KernelPoolView->Create(m_hWnd, rcDefault, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
		WS_EX_CLIENTEDGE);

	m_BigPoolView = new CBigPoolView(m_pFrame);
	m_hwndArray[static_cast<int>(TabColumn::BigPoolTable)] = m_BigPoolView->Create(m_hWnd, rcDefault, NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_OWNERDATA,
		WS_EX_CLIENTEDGE);

	struct {
		PCWSTR Name;
	}columns[] = {
		L"PiDDB Cache",
		L"Unloaded Drivers",
		L"Kernel Pool Tag",
		L"Big Pool",
		L"DPC Timer",
		L"IO Timer"
	};

	int i = 0;
	for (auto& col : columns) {
		m_TabCtrl.InsertItem(i++, col.Name);
	}

	InitPiDDBCacheTable();
	InitUnloadedDriverTable();
	InitDpcTimerTable();
	InitIoTimerTable();

	return 0;
}

LRESULT CKernelView::OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled) {
	CRect rc;
	GetClientRect(&rc);
	int width = rc.Width();
	int clientHeight = rc.Height();
	m_TabCtrl.GetClientRect(&rc);
	int height = rc.Height();
	::MoveWindow(m_TabCtrl.m_hWnd, rc.left, rc.top, width, height, true);

	int iX = rc.left;
	int iY = rc.top + height;
	clientHeight -= height;

	::MoveWindow(m_hwndArray[_index], iX, iY, width, clientHeight, true);

	bHandled = false;
	return 0;
}

LRESULT CKernelView::OnTcnSelChange(int, LPNMHDR hdr, BOOL&) {
	int index = 0;

	index = m_TabCtrl.GetCurSel();
	for (auto hwnd : m_hwndArray) {
		if (::IsWindow(hwnd)) {
			::ShowWindow(hwnd, SW_HIDE);
		}
	}

	switch (static_cast<TabColumn>(index)) {
		case TabColumn::PiDDBCacheTable:
			m_PiDDBCacheTable->ShowWindow(SW_SHOW);
			m_PiDDBCacheTable->SetFocus();
			break;
		case TabColumn::UnloadedDriverTable:
			m_UnloadedDriverTable->ShowWindow(SW_SHOW);
			m_UnloadedDriverTable->SetFocus();
			break;
		case TabColumn::KernelPoolTable:
			m_KernelPoolView->ShowWindow(SW_SHOW);
			m_KernelPoolView->UpdatePaneText();
			break;
		case TabColumn::BigPoolTable:
			m_BigPoolView->ShowWindow(SW_SHOW);
			m_BigPoolView->UpdatePaneText();
			break;
		case TabColumn::DpcTimer:
			m_DpcTimerTable->ShowWindow(SW_SHOW);
			break;
		case TabColumn::IoTimer:
			m_IoTimerTable->ShowWindow(SW_SHOW);
			break;
	}
	_index = index;
	::PostMessage(m_hWnd, WM_SIZE, 0, 0);
	return 0;
}

void CKernelView::InitPiDDBCacheTable() {
	BarDesc bars[] = {
		{22,"驱动名",0},
		{55,"加载状态",0},
		{50,"时间戳/文件哈希",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_PiDDBCacheTable = new CPiDDBCacheTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_PiDDBCacheTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::PiDDBCacheTable)] = m_PiDDBCacheTable->m_hWnd;
	m_PiDDBCacheTable->ShowWindow(SW_SHOW);
}

void CKernelView::InitUnloadedDriverTable() {
	BarDesc bars[] = {
		{22,"驱动名",0},
		{20,"起始地址",0},
		{20,"结束地址",0},
		{40,"卸载时间",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_UnloadedDriverTable = new CUnloadedDriverTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_UnloadedDriverTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::UnloadedDriverTable)] = m_UnloadedDriverTable->m_hWnd;
	m_UnloadedDriverTable->ShowWindow(SW_HIDE);
}

void CKernelView::InitDpcTimerTable() {
	BarDesc bars[] = {
		{22,"定时器对象",0},
		{20,"DPC对象",0},
		{20,"函数入口",0},
		{18,"DueTime",0},
		{10,"触发周期",0},
		{30,"文件厂商",0},
		{260,"函数入口所在内核模块",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_DpcTimerTable = new CDpcTimerTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_DpcTimerTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::DpcTimer)] = m_DpcTimerTable->m_hWnd;
	m_DpcTimerTable->ShowWindow(SW_HIDE);
}

void CKernelView::InitIoTimerTable() {
	BarDesc bars[] = {
		{22,"设备对象",0},
		{20,"Type",0},
		{18,"TimerFlag",0},
		{20,"函数入口",0},
		{30,"文件厂商",0},
		{260,"函数入口所在内核模块",0},
	};

	TableInfo table = {
		1,1,TABLE_SORTMENU | TABLE_COPYMENU | TABLE_APPMENU,9,0,0,0
	};

	BarInfo info;
	info.nbar = _countof(bars);
	info.font = 9;
	for (int i = 0; i < info.nbar; i++) {
		info.bar[i].defdx = bars[i].defdx;
		info.bar[i].mode = bars[i].mode;
		info.bar[i].name = bars[i].name;
	}

	m_IoTimerTable = new CIoTimerTable(info, table);
	RECT rect;
	::GetClientRect(m_TabCtrl.m_hWnd, &rect);
	int height = rect.bottom - rect.top;
	GetClientRect(&rect);
	rect.top += height;
	rect.bottom -= height;
	m_IoTimerTable->Create(m_hWnd, rect, nullptr, WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_EX_LAYERED);
	m_hwndArray[static_cast<int>(TabColumn::IoTimer)] = m_IoTimerTable->m_hWnd;
	m_IoTimerTable->ShowWindow(SW_HIDE);
}

IView* CKernelView::GetCurView() {
	int index = 0;

	index = m_TabCtrl.GetCurSel();
	switch (static_cast<TabColumn>(index)) {
		case TabColumn::PiDDBCacheTable:
			return m_PiDDBCacheTable;
		case TabColumn::UnloadedDriverTable:
			break;
		case TabColumn::KernelPoolTable:
			return m_KernelPoolView;
		case TabColumn::BigPoolTable:
			return m_BigPoolView;
	}

	return nullptr;
}
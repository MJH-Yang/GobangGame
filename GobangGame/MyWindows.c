/* -------------------------------------------------------------------
                    MyWindows.c -- ��������ģ��  
				��Windows ������ƣ�SDK������Ƶ�̳�                    
--------------------------------------------------------------------*/

#include <windows.h>
#include <math.h>

#define BOARD_CELL_NUM 14
#define FIVE_MARK_POINT_RADIUS 4
#define CHESS_PIECE_RADIUS	13
#define BLACK_FLAG 1
#define WHITE_FLAG 2
#define NULL_FLAG 0
#define WIN_CONDITION 5

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("MyWindows");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("���������Ҫ�� Windows NT ����ִ�У�"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, 
		TEXT("�ҵ���������Ϸ"), 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		610, 
		610,
		NULL, 
		NULL, 
		hInstance, 
		NULL);
	
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

// ���ƺ�ɫʵ�ĵ�
HRESULT _DrawBlackSolidPoint(HDC hdc, int radius, POINT postion)
{
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	return S_OK;
}

// ���ư�ɫ���ĵ�
HRESULT _DrawWhiteHollowPoint(HDC hdc, int radius, POINT postion)
{
	SelectObject(hdc, GetStockObject(WHITE_BRUSH));
	Ellipse(hdc, postion.x - radius, postion.y - radius, postion.x + radius, postion.y + radius);

	return S_OK;
}

// ��ȡһС���Ⱥ͸߶�
HRESULT _GetCellWidthAndHeight(POINT ptLeftTop, int cxClient, int cyClient, int *cxCell, int *cyCell)
{
	*cxCell = (cxClient - ptLeftTop.x * 2) / BOARD_CELL_NUM;
	*cyCell = (cyClient - ptLeftTop.y * 2) / BOARD_CELL_NUM;

	return S_OK;
}

// ��ʵ������ת��Ϊ�߼����꣬������Ҫ����ʵ�ʵ㵽���̵��ת��
HRESULT _ExChangeLogicalPosition(POINT actualPostion, POINT ptLeftTop, int cxClient, int cyClient, POINT *logicalPostion)
{
	// ���һС��Ŀ�Ⱥ͸߶�
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// �������Ч��
	if (actualPostion.x < ptLeftTop.x || actualPostion.x > ptLeftTop.x + BOARD_CELL_NUM * cxCell ||
		actualPostion.y < ptLeftTop.y || actualPostion.y > ptLeftTop.y + BOARD_CELL_NUM * cyCell) {
		MessageBox(NULL ,TEXT("�������������壡"), TEXT("��ʾ"), MB_OK);
		return S_FALSE;
	}
	// ��ȡ�����ĸ���
	int xCount = 0, yCount = 0;
	POINT sidePoints[4] = { 0 };
	for (int x = ptLeftTop.x; x <= ptLeftTop.x + BOARD_CELL_NUM * cxCell; x += cxCell, xCount++) {
		if (actualPostion.x >= x && actualPostion.x <= x + cxCell) {
			sidePoints[0].x = x;
			sidePoints[2].x = x;
			sidePoints[1].x = x + cxCell;
			sidePoints[3].x = x + cxCell;
			break;
		}
	}
	for (int y = ptLeftTop.y; y <= ptLeftTop.y + BOARD_CELL_NUM * cyCell; y += cyCell, yCount++) {
		if (actualPostion.y >= y && actualPostion.y <= y + cyCell) {
			sidePoints[0].y = y;
			sidePoints[1].y = y;
			sidePoints[2].y = y + cyCell;
			sidePoints[3].y = y + cyCell;
			break;
		}
	}
	// ���㵱ǰ�㵽�ĸ��㵽��ǰ�����
	double lengthCount[4] = { 0 };
	for (int item = 0; item < 4; ++item) {
		lengthCount[item] = pow(abs(sidePoints[item].x - actualPostion.x), 2) + pow(abs(sidePoints[item].y - actualPostion.y), 2);
	}
	// ��ȡ�ĸ�����ֵ����̵�һ��
	int shortestIndex = 0;
	for (int item = 0; item < 4; ++item) {
		if (lengthCount[item] < lengthCount[shortestIndex]) {
			shortestIndex = item;
		}
	}
	// �����߼����꣬�����±�Ϊ0�ĵ�Ϊ��׼��
	if (1 == shortestIndex) {
		xCount += 1;
	} 
	else if (2 == shortestIndex) {
		yCount += 1;
	}
	else if (3 == shortestIndex) {
		xCount += 1;
		yCount += 1;
	}
	logicalPostion->x = xCount;
	logicalPostion->y = yCount;

	return S_OK;
}

// ���߼�����ת��Ϊʵ������
HRESULT _ExchangeActualPositon(POINT logicalPos, int cxCell, int cyCell, POINT ptLeftTop, POINT *actualPos)
{
	actualPos->x = ptLeftTop.x + logicalPos.x * cxCell;
	actualPos->y = ptLeftTop.y + logicalPos.y * cyCell;

	return S_OK;
}

// ��������
HRESULT DrawChessBoard(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
	// ���һС��Ŀ�Ⱥ͸߶�
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// ��������
	for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
		MoveToEx(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y, NULL);
		LineTo(hdc, ptLeftTop.x + col * cxCell, ptLeftTop.y + BOARD_CELL_NUM * cyCell);
	}
	// ���ƻ�ɫ�ĺ���
	HPEN hPen, hOldPen;
	hPen = CreatePen(PS_SOLID, 1, RGB(190, 190, 190));
	hOldPen = SelectObject(hdc, hPen);
	for (int row = 0; row < 7; ++row) {
		MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + cyCell + row * 2 * cyCell, NULL);
		LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + cyCell + row * 2 * cyCell);
	}
	SelectObject(hdc, hOldPen);
	// ���ƺ�ɫ�ĺ���
	for (int row = 0; row < 8; ++row) {
		MoveToEx(hdc, ptLeftTop.x, ptLeftTop.y + row * 2 * cyCell, NULL);
		LineTo(hdc, ptLeftTop.x + BOARD_CELL_NUM * cxCell, ptLeftTop.y + row * 2 * cyCell);
	}

	return S_OK;
}

// ����������ص�
HRESULT DrawFiveHeavyPoint(HDC hdc, POINT ptLeftTop, int cxClient, int cyClient)
{
	// ���һС��Ŀ�Ⱥ͸߶�
	int cxCell = 0, cyCell = 0;
	_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
	// ���߼���ת��Ϊʵ�ʵ�
	POINT logicalPoint[5] = { 3, 3, 3, 11, 11, 3, 11, 11, 7, 7 };
	POINT actualPoint[5] = { 0 };
	for (int cPt = 0; cPt < 5; ++cPt) {
		_ExchangeActualPositon(logicalPoint[cPt], cxCell, cyCell, ptLeftTop, &actualPoint[cPt]);
	}
	// ���������ɫʵ�ĵ�
	for (int cPt = 0; cPt < 5; ++cPt) {
		_DrawBlackSolidPoint(hdc, FIVE_MARK_POINT_RADIUS, actualPoint[cPt]);
	}

	return S_OK;
}

// �ж��Ƿ�ʤ��
HRESULT IsSomeoneWin(int chessPoints[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1], int *winner)
{
	// �����������ʤ��
	int blackHrozCount = 0;
	int whiteHrozCount = 0;
	// �����������ʤ��
	int blackVrclCount = 0;
	int whiteVrclCount = 0;
	// б���������ʤ��

	// �����������ŵ�ʤ��
	for (int row = 0; row < BOARD_CELL_NUM + 1; ++row) {
		for (int col = 0; col < BOARD_CELL_NUM + 1; ++col) {
			
		}
		if (WIN_CONDITION == blackVrclCount || WIN_CONDITION == blackHrozCount) {
			*winner = BLACK_FLAG;
			break;
		}
		if (WIN_CONDITION == whiteVrclCount || WIN_CONDITION == whiteHrozCount) {
			*winner = WHITE_FLAG;
			break;
		}
	}


	return S_OK;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	// �ͻ�������
	static int cxClient, cyClient;
	// ÿ������Ŀ�Ⱥ͸߶�
	int cxCell = 0, cyCell = 0;
	// �������Ͻ�����
	POINT ptLeftTop;
	ptLeftTop.x = 30;
	ptLeftTop.y = 40;
	// �����ʵ�ʵ�
	POINT actualPosition;
	// �����ʵ�ʵ����ת�������߼���
	POINT logicalPostion;
	// �����ʵ�ʵ����ת�������߼����Ӧ��ʵ�ʵ�
	POINT changedActualPosition;
	// ��¼�߼�λ�õ����飬����BLACK_FLAGΪ���ӣ�WHITE_FLAGΪ���ӣ�NULL_FLAGΪ�հ�
	static int chessPoints[BOARD_CELL_NUM + 1][BOARD_CELL_NUM + 1] = { NULL_FLAG };
	// ʤ����
	int winner = NULL_FLAG;
	// ��������ֵ
	HRESULT hResult = S_FALSE;

	switch (message)
	{
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;

	case WM_LBUTTONDOWN:	// ����
		// ���ʵ�ʵ�
		actualPosition.x = LOWORD(lParam);
		actualPosition.y = HIWORD(lParam);
		// ��ö�Ӧ�ļ��������߼���
		hResult = _ExChangeLogicalPosition(actualPosition, ptLeftTop, cxClient, cyClient, &logicalPostion);
		if (S_FALSE == hResult) {
			return 0;
		}
		// ���߼����¼����
		chessPoints[logicalPostion.x][logicalPostion.y] = BLACK_FLAG;
		// ���һС��Ŀ�Ⱥ͸߶�
		_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
		// ���߼���ת��Ϊʵ�ʵ�
		POINT changedActualPosition;
		_ExchangeActualPositon(logicalPostion, cxCell, cyCell, ptLeftTop, &changedActualPosition);
		// ����ʵ�ʵ�
		hdc = GetDC(hwnd);
		_DrawBlackSolidPoint(hdc, CHESS_PIECE_RADIUS, changedActualPosition);
		ReleaseDC(hwnd, hdc);
		// ����ʤ��
		IsSomeoneWin(chessPoints, &winner);
		if (BLACK_FLAG == winner) {
			MessageBox(hwnd, TEXT("�����ʤ��"), TEXT("��ʾ"), NULL);
		}
		if (WHITE_FLAG == winner) {
			MessageBox(hwnd, TEXT("�����ʤ��"), TEXT("��ʾ"), NULL);
		}
		return 0;

	case WM_RBUTTONDOWN:	// ����
		// ���ʵ�ʵ�
		POINT actualPosition, logicalPostion;
		actualPosition.x = LOWORD(lParam);
		actualPosition.y = HIWORD(lParam);
		// ��ö�Ӧ�ļ��������߼���
		hResult = _ExChangeLogicalPosition(actualPosition, ptLeftTop, cxClient, cyClient, &logicalPostion);
		if (S_FALSE == hResult) {
			return 0;
		}
		// ���߼����¼����
		chessPoints[logicalPostion.x][logicalPostion.y] = WHITE_FLAG;
		// ���һС��Ŀ�Ⱥ͸߶�
		_GetCellWidthAndHeight(ptLeftTop, cxClient, cyClient, &cxCell, &cyCell);
		// ���߼���ת��Ϊʵ�ʵ�
		_ExchangeActualPositon(logicalPostion, cxCell, cyCell, ptLeftTop, &changedActualPosition);
		// ����ʵ�ʵ�
		hdc = GetDC(hwnd);
		_DrawWhiteHollowPoint(hdc, CHESS_PIECE_RADIUS, changedActualPosition);
		ReleaseDC(hwnd, hdc);
		// ����ʤ��
		IsSomeoneWin(chessPoints, &winner);
		if (BLACK_FLAG == winner) {
			MessageBox(hwnd, TEXT("�����ʤ��"), TEXT("��ʾ"), NULL);
		}
		if (WHITE_FLAG == winner) {
			MessageBox(hwnd, TEXT("�����ʤ��"), TEXT("��ʾ"), NULL);
		}
		return 0;

	case WM_MBUTTONDOWN:	// ��ʼ������
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// ��ʼ������
		DrawChessBoard(hdc, ptLeftTop, cxClient, cyClient);			// ��������
		DrawFiveHeavyPoint(hdc, ptLeftTop, cxClient, cyClient);		// ����������ص�

		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}
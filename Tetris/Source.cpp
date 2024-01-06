#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

int nScreenWidth;			// Console Screen Size X (columns)
int nScreenHeight;			// Console Screen Size Y (rows)
wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char *pField = nullptr;

int Rotate(int px,int py, int r)
{	int pindex;
	switch (r%4)
	{
	case 0:
		pindex = 4*py+px;
		break;
	case 1:
		pindex = 12+py-(px*4);
		break;
	case 2:
		pindex = 15-4*py-px;
		break;
	case 3:
		pindex = 3-py +(px*4);
		break;
	}
	return pindex;
}
bool DoesPieceFit(int nTetromino, int nRotation, int nPosx,int nPosy)
{
	for(int px=0;px<4;px++ )
		for(int py=0;py<4;py++)
		{
			int pi=Rotate(px,py,nRotation);

			int fi = (nPosy+py)*nFieldWidth+(nPosx+px);

			if(nPosx+px>=0 && nPosx+px<nFieldWidth)
				if(nPosy+py>0 && nPosy+py<nFieldHeight)
				{
					if(tetromino[nTetromino][pi] != L'.' && pField[fi]!=0)
						return false;

				}
		}
	return true;
}
int main() {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
  
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    nScreenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    nScreenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

	std::wstring screen=L" ";

	for (int i = 0; i < nScreenWidth*nScreenHeight; i++) 
		screen+=L" ";


	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;
	//Shapes
	tetromino[0].append(L"..X...X...X...X."); // Tetromino 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldWidth*nFieldHeight]; // Create play field buffer
	for (int x = 0; x < nFieldWidth; x++) // Board Boundary
		for (int y = 0; y < nFieldHeight; y++)
			pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;

	bool bGameOver=false;
	
	bool bKey[4];
	int nCurrentPiece = 0;
	int nCurrentRotation = 0;
	int nCurrentX = nFieldWidth / 2;
	int nCurrentY = 0;
	int nSpeed = 20;
	int nSpeedCount = 0;
	bool bForceDown = false;
	bool bRotateHold = true;
	int nPieceCount = 0;
	int nScore = 0;
	vector<int> vLines;


	while(!bGameOver){
		//Timing
		this_thread::sleep_for(50ms);
		nSpeedCount++;
		bForceDown=(nSpeedCount==nSpeed);
		//Input
		for(int k=0;k<4;k++)
			bKey[k]=(0x8000 & GetAsyncKeyState((unsigned char)("DASR"[k]))) != 0;


		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;		
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		if (bKey[3])
		{
			nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = false;
		}
		else {
			bRotateHold = true;
		}
		if(bForceDown)
		{
			nSpeedCount = 0;
			nPieceCount++;
			if (nPieceCount % 50 == 0)
				if (nSpeed >= 10) nSpeed--;

			if(DoesPieceFit(nCurrentPiece,nCurrentRotation,nCurrentX,nCurrentY+1))
				nCurrentY++;
			else
			{
				//Lock
				for (int px = 0; px < 4; px++)
					for (int py = 0; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;



				//Lines
				for (int py = 0; py < 4; py++)
					if(nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine = true;
						for (int px = 1; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							// Remove Line, set to =
							for (int px = 1; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;
							vLines.push_back(nCurrentY + py);
						}						
					}

				nScore += 25;
				if(!vLines.empty())	nScore += (1 << vLines.size()) * 100;

				//choose next
				this_thread::sleep_for(10ms);
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				//game over
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

			}
		}
		//SCore
		swprintf_s(&screen[nScreenWidth + nFieldWidth + 10], 16, L"SCORE: %8d", nScore);

		//Draw Field
		for (int x = 0; x < nFieldWidth; x++)
			for (int y = 0; y < nFieldHeight; y++)
				screen[(y + 2)*nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y*nFieldWidth + x]];
		//Draw Current Piece
		for(int px=0;px<4;px++)
			for(int py=0; py<4; py++)
				if(tetromino[nCurrentPiece][Rotate(px,py,nCurrentRotation)]==L'X')
					screen[(nCurrentY + py+2)*nScreenWidth+(nCurrentX+px+2)]=nCurrentPiece+65;
		//Line Completion
		if (!vLines.empty())
		{
			// Display Frame (cheekily to draw lines)
			WriteConsoleOutputCharacter(hConsole, screen.c_str(), nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			this_thread::sleep_for(400ms); // Delay a bit

			for (auto &v : vLines)
				for (int px = 1; px < nFieldWidth - 1; px++)
				{
					for (int py = v; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}

			vLines.clear();
		}


		//Display Frame
		WriteConsoleOutputCharacter(hConsole, screen.c_str(), nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	CloseHandle(hConsole);
	cout<<"Game Over"; 
	return 0;
}
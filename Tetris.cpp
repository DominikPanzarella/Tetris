/*
*
* ---------------------------------------------------------------------------------------
* ---------------------------------------------------------------------------------------
*
* Tetris Games
*
* ---------------------------------------------------------------------------------------
*
* Author : Dominik Panzarella
*
* Date : 21.06.2023
*
* Version : 1.0
* 
* ---------------------------------------------------------------------------------------
*
* Credit to https ://www.onelonecoder.com
*
* ---------------------------------------------------------------------------------------
* ---------------------------------------------------------------------------------------
*/
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>

wstring tetromino[7];

int nScreenWidth = 80;			// Console Screen Size X (columns)
int nScreenHeight = 30;			// Console Screen Size Y (rows)

//Playing field
int nFieldWidth = 12;           //Console Screen Size X (columns)
int nFieldHeight = 18;          //Console Screen Size X (rows)
unsigned char* pField = nullptr;


int Rotate(int px, int py, int r)
{
    switch (r % 4)
    {
        case 0: return py * 4 + px;             //0 degrees
        case 1: return 12 + py - (px * 4);      //90 degrees
        case 2: return 15 - (py * 4) - px;      //180 degrees
        case 3: return 3 - py + (px * 4);       //270 degrees
    }
    return 0;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosx, int nPosy)
{
    for (int px = 0; px < 4; px++)
    {
        for (int py = 0; py < 4; py++)
        {
            //Get index into piece
            int pi = Rotate(px, py, nRotation);

            //Get index into field
            int fi = (nPosy + py) * nFieldWidth + (nPosx + px);
            
            if (nPosx + px >= 0 && nPosx + px < nFieldWidth)
            {
                if (nPosy + py >= 0 && nPosy + py < nFieldHeight)
                {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0)
                        return false;       //fail on first hit 
                }
            }
        }
    }
    return true;
}

int main()
{       

    // Create Screen Buffer
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
        
    tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");
   
    pField = new unsigned char[nFieldWidth * nFieldHeight];
    for (int x = 0; x < nFieldWidth; x++)
        for (int y = 0; y < nFieldHeight; y++)
            pField[y * nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight - 1) ? 9 : 0;



    bool bGameOver = false;

    int nCurrentPiece = 0;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = true;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    vector<int> vLines;

    while (!bGameOver)      //Main Loop
    {

        //Game Timing
        this_thread::sleep_for(50ms);
        nSpeedCounter++;
        bForceDown = (nSpeedCounter == nSpeed);

        //Input
        for (int k = 0; k < 4; k++)								// R   L   D Z
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

        //Game Logic


        nCurrentX += (bKey[0] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))) ? 1 : 0;
        nCurrentX += (bKey[1] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))) ? -1 : 0;
        nCurrentY += (bKey[2] && (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1))) ? 1 : 0;
        

        if (bKey[3])
        {
            nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = false;
        }
        else
            bRotateHold = true;

        if (bForceDown)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
                nCurrentY++;        //It can, so do it!
            else {
                //Lock the current piece in the field
                for (int px = 0; px < 4; px++)
                    for (int py = 0; py < 4; py++)
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                
                nPieceCount++;
                if (nPieceCount % 10 == 0)
                    if (nSpeed >= 10) nSpeed--;

                //Check have we got any lines
                for(int py=0; py<4; py++)
                    if (nCurrentY + py < nFieldHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++)
                            bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

                        if (bLine)
                        {
                            //Remove Line, set to = 
                            for (int px = 1; px < nFieldWidth - 1; px++)
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;

                            vLines.push_back(nCurrentY + py);
                        }
                    }

                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

                //Choose next piece
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                //If piece does not fit 
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

            }
            nSpeedCounter = 0;
        }

        //Render Output

        //Draw Field
        for(int x=0; x<nFieldWidth; x++)
            for(int y=0; y<nFieldHeight; y++)
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];


        //Draw Current Piece
        for(int px=0; px<4; px++)
            for(int py=0;py<4; py++)
                				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
					screen[(nCurrentY + py + 2)*nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);


        if (!vLines.empty())
        {
            //Display Frame (cheekily to draw lines)
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(40ms);       //Delay a bit

            for (auto& v : vLines)
                for (int px = 1; px < nFieldWidth - 1; px++)
                {
                    for (int py = v; py > 0; py--)
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    pField[px] = 0;
                }

            vLines.clear();
        }

        //Display frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
    }

    //Game over :(
    CloseHandle(hConsole);
    cout << "Game Over!! Score:" << nScore << endl;
    system("pause");

    return 0;
}


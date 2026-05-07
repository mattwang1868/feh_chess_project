
/*
Chess!
*/

/* 
CONTENTS:

Definitions: line 22
Classes: line 90
    IMPORTANT: Board Class: line 161
Function prototypes: line
Main Funtction: line
Funtion Definitions: line
    Main Menu Functions: line
    Game Options Menu Functions: line
    Game Functions: line
        IMPORTANT: Move function: line

*/

#define XMAX 319
#define YMAX 239
#define MenuWidth 200
#define MenuHeight 35
#define SQUARE_LENGTH 23
#define TOP_LEFT_BOARD_X 30
#define TOP_LEFT_BOARD_Y 30
#define MAXTIME 300
#define CIRCLE_R 8
#define TIMER_X 107
#define TIMER_WHITE_Y 220
#define TIMER_BLACK_Y 10
#define TIMER_BOX_X 32
#define TIMER_BOX_Y 15
#define END_SCREEN_LENGTH 80
#define BORDERWIDTH 3

//image offsets (for piece placements)
#define XPAWNOFFSET -4
#define XROOKOFFSET -5
#define XBISHOPOFFSET -4
#define XKNIGHTOFFSET -2
#define XQUEENOFFSET -4
#define XKINGOFFSET -4

#define YPAWNOFFSET 0
#define YROOKOFFSET -3
#define YBISHOPOFFSET -2
#define YKNIGHTOFFSET -2
#define YQUEENOFFSET 0
#define YKINGOFFSET -3

//for piece movements
#define RIGHT {1,0}
#define UP {0,-1}
#define DOWN {0,1}
#define LEFT {-1,0}
#define DOWNRIGHT {1,1}
#define UPLEFT{-1,-1}
#define DOWNLEFT{-1,1}
#define UPRIGHT{1,-1}
#define KNIGHT_L1 {2,1}
#define KINIGHT_L2 {1,2}

#include "FEHLCD.h"
#include "FEHUtility.h"
#include "FEHImages.h"
#include "FEHRandom.h"
#include <math.h>
#include <string.h>

//GLOBAL: stats tracking
int totalmoves = 0;
int gamesplayed = 0;
int piecescaptured = 0;
float timeplayed = 0;

//Random Number Generator
int randomInRange(int low, int high) {
    int range = high - low;
    return low + (Random.RandInt() % range);
}



/*
*******
*******
CLASSES
*******
*******
*/



class timer //class that starts, resets, and runs timer
{
    public:
        char color;
        bool activated;
        float timeLeft;
        bool isComputer;
        timer(char c, bool comp) {
            color = c;
            activated = false;
            timeLeft = MAXTIME; //sets timer to 5 minutes
            isComputer = comp; //used to determine if a player should call random move function
        }
        //in case a function call is needed that would otherwise decrement the timer
        void setActivated(){
            activated = true;
        }
        //removes 0.05 seconds from the timer and adds it to total time played
        void passTwentiethSecond() {
            if(activated) {
                Sleep(0.05);
                timeLeft -= 0.05;
                timeplayed += 0.05;
            }
        }
        //used to determine if a player ran out of time
        bool timeOver() {
            return timeLeft <= 0;
        }
        //displays the current time on the timer
        void displayTimer() {
            //msg is a string that is set to the current time on the timer in a mm:ss format
            int minutes = (int)timeLeft /60;
            int seconds = (int)timeLeft % 60;
            char msg[6];
            msg[0] = '0' + (minutes / 10);
            msg[1] = '0' + (minutes % 10);
            msg[2] = ':';
            msg[3] = '0' + (seconds / 10);
            msg[4] = '0' + (seconds % 10);
            msg[5] = '\0';
            int x,y;
            //TIMER_X, and TIMER_WHITE_Y/TIMER_BLACK_Y are constants for the position of the timer
            x = TIMER_X;
            //create a white rectangle in which the timer is displayed
            if(color == 'w') {
                LCD.SetFontColor(WHITE);
                LCD.FillRectangle(TIMER_X,TIMER_WHITE_Y,TIMER_BOX_X,TIMER_BOX_Y);

                y = TIMER_WHITE_Y;
            }
            else {
                LCD.SetFontColor(WHITE);
                LCD.FillRectangle(TIMER_X,TIMER_BLACK_Y,TIMER_BOX_X,TIMER_BOX_Y);
                y = TIMER_BLACK_Y;
            }
            //set font color to red and display the timer
            LCD.SetFontColor(RED);
            LCD.WriteAt(msg, x,y);

        }
};


class board //board class stores the current position on the board
{
    public:
        char boardState[8][8]; //chracter aray storing the board state
        bool legalState[8][8]; //tells us what squares a piece can move to
        bool attackedState[8][8]; //tells us what squares are under attack - dependent on what turn it is
        bool enPassantable[8][8]; //yeah it's only true on squares where there's a pawn who moved forward 2 squares
        //to track castling rights
        bool whiteKingMoved;
        bool blackKingMoved;
        bool whiteKingRookMoved;
        bool blackKingRookMoved;
        bool whiteQueenRookMoved;
        bool blackQueenRookMoved;
        //sets initial board state to blank
        board() {
            for(int i = 0; i<8;i++) {
                for(int j = 0; j<8; j++) {
                    boardState[i][j] = ' ';
                    legalState[i][j] = false;
                    enPassantable[i][j] = false;
                    attackedState[i][j] = false;
                }
            }
            fillStartingPieces(); //call function to fill initial piece positions
            whiteKingMoved=false;
            blackKingMoved = false;
            blackKingRookMoved = false;
            whiteKingRookMoved = false;
            whiteQueenRookMoved = false;
            blackQueenRookMoved = false;
        }
        
        //copies a board's data in
        board(char position[8][8]) {
            for(int i = 0; i<8;i++) {
                for(int j = 0; j<8; j++) {
                    boardState[i][j] = position[i][j];
                    legalState[i][j] = false;
                    enPassantable[i][j] = false;
                    attackedState[i][j] = false;
                }
            }
            //the following should theoretically not be used by the program anytime this constructor is used
            whiteKingMoved=false;
            blackKingMoved = false;
            blackKingRookMoved = false;
            whiteKingRookMoved = false;
            whiteQueenRookMoved = false;
            blackQueenRookMoved = false;
        }
        //copies more of the board's data in
        //Unused
        board(char position[8][8], bool wkm, bool bkm, bool wkrm, bool bkrm, bool wqrm, bool bqrm) {
            for(int i = 0; i<8;i++) {
                for(int j = 0; j<8; j++) {
                    boardState[i][j] = position[i][j];
                    legalState[i][j] = false;
                    enPassantable[i][j] = false;
                    attackedState[i][j] = false;
                }
            }
            //used in simulate move to determine move legality
            whiteKingMoved=wkm;
            blackKingMoved = bkm;
            blackKingRookMoved = wkrm;
            whiteKingRookMoved = bkrm;
            whiteQueenRookMoved = wqrm;
            blackQueenRookMoved = bqrm;
        }

        //fills pieces into their initial positions with loops
        void fillStartingPieces() {
            //back rows
            char pieceOrderWhite[] = {'R','N','B','Q','K','B','N','R'}; //characters denote pieces, will be converted to sprites later
            char pieceOrderBlack[] = {'r','n','b','q','k','b','n','r'};
            //populate pieces
            for(int i = 0; i < 8; i++) {
                //rows and columns are switched so representing the board as an array, it looks sideways, which makes it easier as a chess player
                //1st element is the column of the board, second is row - like in chess notation
                boardState[i][1] = 'p';
                boardState[i][6] = 'P';
                boardState[i][0] = pieceOrderBlack[i];
                boardState[i][7] = pieceOrderWhite[i];
            }
        }

        //board.makeMove changes the boardstate to the second pair of file/rank coordinates
        void makeMove(int file1, int rank1, int file2, int rank2) {
            //swap to new board state using temporary variable
            char temp = boardState[file1][rank1];
            //if a rook or king moved, change the respective variable for castling calculation
            if(temp == 'R') {
                if(file1 == 0 && rank1 ==7) {
                    whiteQueenRookMoved = true;
                }
                else if(file1 == 7 && rank1 == 7) {
                    whiteKingRookMoved = true;
                }
            }
            else if (temp == 'r') {
                if(file1 == 0 && rank1 ==0) {
                    blackQueenRookMoved = true;
                }
                else if(file1 == 7 && rank1 == 0) {
                    blackKingRookMoved = true;
                }
            }
            else if (temp == 'k') {
                blackKingMoved = true;
            }
            else if (temp == 'K') {
                whiteKingMoved = true;
            }
            //account for pawn promotion - auto-queen for simplicity
            else if(temp == 'p' && rank2 == 7) {
                temp = 'q';
            }
            else if(temp == 'P' && rank2 == 0) {
                temp = 'Q';
            }
            //finish the swap function
            //conveniently enough, this also clears any pieces that would be captured
            boardState[file1][rank1] = ' ';
            boardState[file2][rank2] = temp;
        }
        
        //sets legalState to wherever the rook on (file,rank) would be able to go
        void validRookMoves(int file, int rank) {
            //movement vectors for where a rook can move
            int movements[4][2] = {RIGHT, LEFT, UP, DOWN};
            //find if the rook is white
            bool isWhite = isupper(boardState[file][rank]);
            //loop through each movement vector
            for(int i = 0; i<4; i++) {
                int moveVector[2] = {movements[i][0], movements[i][1]};
                //searchFile and searchRank are where we will look to see if that specific square is legal
                int searchFile = file + moveVector[0];
                int searchRank = rank + moveVector[1];
                //rooks can move to the edge of the board or until they hit a piece
                while ((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' || (isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    //conditions met - set the legalState to true- meaning the rook can move there
                    legalState[searchFile][searchRank] = true;
                    //this case only happens if the searched square is an enemy piece. We want to not go further from there
                    if(boardState[searchFile][searchRank] != ' ') {
                        break;
                    }
                    //change searchFile and searchRank according to movement vector
                    searchFile += moveVector[0];
                    searchRank += moveVector[1];
                }
            }
        }
        
        //finds all squares defended by the rook on (file,rank)
        //similar to validRookMoves as it finds anywhere the rook could attack, but it also includes white pieces in the path
        void attackedRookSquares(int file, int rank) {
            int movements[4][2] = {RIGHT, LEFT, UP, DOWN};
            for(int i = 0; i<4; i++) {
                int moveVector[2] = {movements[i][0], movements[i][1]};
                int searchFile = file + moveVector[0];
                int searchRank = rank + moveVector[1];
                
                while ((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8)) {
                    attackedState[searchFile][searchRank] = true;
                    if(boardState[searchFile][searchRank] != ' ') {
                        break;
                    }
                    searchFile += moveVector[0];
                    searchRank += moveVector[1];
                }
            }
        }
        //similar to validRookMoves - but the movements are diagonal
        void validBishopMoves(int file, int rank) {
            int movements[4][2] = {DOWNRIGHT, UPRIGHT, DOWNLEFT, UPLEFT};
            bool isWhite = isupper(boardState[file][rank]);
            int searchFile, searchRank;
            int moveVector[2];
            for(int i = 0; i<4; i++) {
                moveVector[0] = movements[i][0];
                moveVector[1] = movements[i][1];
                searchFile = file + moveVector[0];
                searchRank = rank + moveVector[1];
                while ((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' || (isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    legalState[searchFile][searchRank] = true;
                    if(boardState[searchFile][searchRank] != ' ') {
                        break;
                    }
                    searchFile += moveVector[0];
                    searchRank += moveVector[1];

                }
            }
        }
        //similar to validBishopMoves - but includes white pieces in the path
        void attackedBishopSquares(int file, int rank) {
            int movements[4][2] = {DOWNRIGHT, UPRIGHT, DOWNLEFT, UPLEFT};
            for(int i = 0; i<4; i++) {
                int moveVector[2] = {movements[i][0], movements[i][1]};
                int searchFile = file + moveVector[0];
                int searchRank = rank + moveVector[1];
                
                while ((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8)) {
                    attackedState[searchFile][searchRank] = true;
                    if(boardState[searchFile][searchRank] != ' ') {
                        break;
                    }
                    searchFile += moveVector[0];
                    searchRank += moveVector[1];
                }
            }
        }
        //the queen moves in a combination of the rook and bishop - so calling both will give the ways the queen can move - since legalState is not reset
        void validQueenMoves(int file, int rank) {
            validBishopMoves(file, rank);
            validRookMoves(file, rank);
        }
        //same logic as validQueenMoves
        void attackedQueenSquares(int file, int rank) {
            attackedBishopSquares(file, rank);
            attackedRookSquares(file,rank);
        }
        //finds legal Knight Moves
        void validKnightMoves(int file, int rank) {
            //Knights move either {2,1} or {1,2} then rotated in 90 degrees in all possible ways
            //ie {+-2,+-1} and {+-1,+-2} for any combination of + and -
            int movements[2][2] = {KNIGHT_L1, KINIGHT_L2};
            //for each in movements
            for(int i = 0; i<2; i++) {
                int searchFile = file; 
                int searchRank = rank;
                bool isWhite = isupper(boardState[file][rank]);
                searchFile += movements[i][0];
                searchRank += movements[i][1];
                //find if the knight can move to this square
                if((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' || (isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    legalState[searchFile][searchRank] = true;
                }
                //reflect across vertical axis
                searchFile -= 2*movements[i][0];
                if((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' || (isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    legalState[searchFile][searchRank] = true;

                }
                //reflect across horizontal axis
                searchRank -= 2*movements[i][1];
                if((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' || (isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    legalState[searchFile][searchRank] = true;
                }
                //reflect back across vertical axis
                searchFile += 2*movements[i][0];
                if((searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) && (boardState[searchFile][searchRank] == ' ' ||(isupper(boardState[searchFile][searchRank])!=isWhite))) {
                    legalState[searchFile][searchRank] = true;
                }
            }
        }
        //similar to validKnightMoves, but pieces on the square considered are not considered
        void attackedKnightSquares(int file, int rank) {
            int movements[2][2] = {KNIGHT_L1, KINIGHT_L2};
            for(int i = 0; i<2; i++) {
                int searchFile = file; 
                int searchRank = rank;
                searchFile += movements[i][0];
                searchRank += movements[i][1];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
                searchFile -= 2*movements[i][0];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
                searchRank -= 2*movements[i][1];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
                searchFile += 2*movements[i][0];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
            }
        }
        //finds legal pawn moves
        void validPawnMoves(int file, int rank) {   
            //pawns are weird - they can move up but only capture diagonally         
            int movements[3][2] = {UP, UPRIGHT, UPLEFT};
            bool isWhite = isupper(boardState[file][rank]);
            int searchFile=file, searchRank=rank;
            //black pawns are on the opposite side, so the up needs to be reversed to a down
            if(!isWhite) {
                int down[] = DOWN;
                int dr[] = DOWNRIGHT;
                int dl[] = DOWNLEFT;
                movements[0][0] = down[0];
                movements[0][1] = down[1];
                movements[1][0] = dr[0];
                movements[1][1] = dr[1];
                movements[2][0] = dl[0];
                movements[2][1] = dl[1];
            }
            //vector for vertical pawn movement
            int verticalVector[] = {movements[0][0], movements[0][1]};
            //search the square immediately above/below the pawn
            searchFile += verticalVector[0];
            searchRank += verticalVector[1];
            if((searchFile >=0 && searchFile <8 && searchRank >=0 &&searchRank <8) && boardState[searchFile][searchRank] == ' ') {
                //if the pawn can move forward without anything blocking the way, set legalState to true
                legalState[searchFile][searchRank] = true;
                //if the pawns are on their starting positions, they can move forward 2 squares, so repeat the process again
                if((isWhite && rank == 6) || (!isWhite && rank == 1)) {
                    searchFile += verticalVector[0];
                    searchRank += verticalVector[1];
                    if(boardState[searchFile][searchRank] == ' ') {
                        legalState[searchFile][searchRank] = true;
                    }
                }
            }
            //check the diagonals
            for(int i = 1; i<3; i++) {
                searchFile = file; searchRank = rank;
                int moveVector[2] = {movements[i][0], movements[i][1]};
                searchFile += moveVector[0];
                searchRank += moveVector[1];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    //checks if the square has an enemy piece
                    if(isupper(boardState[searchFile][searchRank]) != isWhite && boardState[searchFile][searchRank] != ' ') {
                        legalState[searchFile][searchRank] = true;
                    }
                }
            }
            //en passant check
            if(isWhite && rank == 3) {
                //checks if the enemy pawns directly next to the pawn have move forward 2 squares last turn
                if(file-1 >=0 && boardState[file-1][rank] == 'p' && enPassantable[file-1][rank]) {
                    legalState[file-1][rank-1] = true;
                }
                if(file+1 <8 && boardState[file+1][rank] == 'p' && enPassantable[file+1][rank]) {
                    legalState[file+1][rank-1] = true;
                }
            }
            else if(rank == 4 && !isWhite){
                if(file+1 <8 && boardState[file+1][rank] == 'P' && enPassantable[file+1][rank]) {
                    legalState[file+1][rank+1] = true;
                }
                if(file-1 >=0 && boardState[file-1][rank] == 'P' && enPassantable[file-1][rank]) {
                    legalState[file-1][rank+1] = true;
                }
            }
        }
        //finds attacked pawn squares
        void attackedPawnSquares(int file, int rank) {
            //only diagonals need be considered since pawns can't capture forward
            bool isWhite = isupper(boardState[file][rank]);
            int movements[2][2] = {UPRIGHT, UPLEFT};
            if(!isWhite) {
                int dr[2] = DOWNRIGHT;
                int dl[2] = DOWNLEFT;
                movements[0][0] = dr[0];
                movements[0][1] = dr[1];
                movements[1][0] = dl[0];
                movements[1][1] = dl[1];
            }
            int searchFile, searchRank;
            //finds if the diagonals are in bounds then adds them to attackedState
            for(int i = 0; i<2; i++) {
                searchFile = file; searchRank = rank;
                int moveVector[2] = {movements[i][0], movements[i][1]};
                searchFile += moveVector[0];
                searchRank += moveVector[1];
                if(searchFile >=0 && searchFile <8 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
            }
        }
        //finds valid king moves
        void validKingMoves(int file,int rank) {
            //kings can move 1 square in any direction
            int movements[8][2] = {UP,UPLEFT,LEFT, DOWNLEFT,DOWN,DOWNRIGHT, RIGHT, UPRIGHT};
            bool isWhite = isupper(boardState[file][rank]);
            //call function that sets attacked state to all of the attacked squares the king wouldn't be able to move to
            findEnemyAttackedSquares(isWhite);
            //search all squares the king can move to
            for(int i = 0; i<8; i++) {
                int movementVector[2] = {movements[i][0],movements[i][1]};
                int searchFile = file + movementVector[0];
                int searchRank = rank + movementVector[1];
                //if it's in bounds
                if(searchFile < 8 && searchFile >=0 && searchRank >=0 && searchRank <8) {
                    //check that the square doesn't have a same color piece on it as well as it not being attacked by the enemy
                    if((boardState[searchFile][searchRank] ==' ' || isWhite != isupper(boardState[searchFile][searchRank])) && !attackedState[searchFile][searchRank]) {
                        legalState[searchFile][searchRank] = true;
                    }
                }
            }
            //check if the conditions are met for castling
            if(canCastleKSide(isWhite)) {
                //sets the square g1 as legal for castling
                legalState[file+2][rank] = true;
            }
            if(canCastleQSide(isWhite)) {
                //sets the square c1 as legal for castling
                legalState[file-2][rank] = true;
            }
        }
        //finds squares attacked by the king
        //similar to validKingMoves, except all squares adjacent to the king are added to attackedState
        void attackedKingSquares(int file, int rank) {
            int movements[8][2] = {UP,UPLEFT,LEFT, DOWNLEFT,DOWN,DOWNRIGHT, RIGHT, UPRIGHT};
            for(int i = 0; i<8; i++) {
                int movementVector[2] = {movements[i][0],movements[i][1]};
                int searchFile = file + movementVector[0];
                int searchRank = rank + movementVector[1];
                if(searchFile < 8 && searchFile >=0 && searchRank >=0 && searchRank <8) {
                    attackedState[searchFile][searchRank] = true;
                }
            }
        }
        //sets attackedState to true in all squares the opposite color is attacking
        void findEnemyAttackedSquares(bool isWhite) {
            //reset the attackedState to prevent carrying over values
            resetAttackedState();
            //find all black pieces and find their attacked squares, updating attackedState
            if(isWhite) {
                for(int i = 0; i < 8; i++) {
                    for(int j = 0; j<8; j++) {
                        if(islower(boardState[i][j])) {
                            findAttackedSquares(i,j);
                        }
                    }
                }
            }
            //find all white pieces and find their attacked squares, updating attackedState
            else {
                for(int i = 0; i < 8; i++) {
                    for(int j = 0; j<8; j++) {
                        if(isupper(boardState[i][j])) {
                            findAttackedSquares(i,j);
                        }
                    }
                }
            }
        }
        //updates attackedState to true anywhere the piece on file,rank is attacking
        void findAttackedSquares(int file, int rank) {
            char piece = boardState[file][rank];
            //pawn - find squares pawn is attacking
            if(piece == 'P' || piece == 'p') {
                attackedPawnSquares(file, rank);
            }
            //rook
            else if(toupper(piece) == 'R') {
                attackedRookSquares(file,rank);
            }
            //knight
            else if(piece == 'N' || piece == 'n') {
                attackedKnightSquares(file,rank);
            }
            //bishop
            else if(toupper(piece) == 'B') {
                attackedBishopSquares(file, rank);
            }
            //queen
            else if(toupper(piece)=='Q') {
                attackedQueenSquares(file,rank);
            }
            //king
            else if(toupper(piece)=='K') {
                attackedKingSquares(file,rank);
            }
            //should never happen
            else if(piece == ' ') {
                LCD.Write("ERROR: INVALIDPIECEYOU'REDUMB");
            }
            //also should never happen
            else {
                resetLegalState();
                LCD.Write(piece+" HOWDIDTHISGETHERE");
            }
        }
        
        //sets legalState to legal moves of each piece
        //same as findAttackedSquares but calls valid(Piece)Moves instead
        void findLegalMoves(int file, int rank) {
            char piece = boardState[file][rank];
            if(piece == 'P' || piece == 'p') {
                validPawnMoves(file, rank);
            }
            else if(toupper(piece) == 'R') {
                validRookMoves(file,rank);
            }
            else if(piece == 'N' || piece == 'n') {
                validKnightMoves(file,rank);
            }
            else if(toupper(piece) == 'B') {
                validBishopMoves(file, rank);
            }
            else if(toupper(piece)=='Q') {
                validQueenMoves(file,rank);
            }
            else if(toupper(piece)=='K') {
                validKingMoves(file,rank);
            }
            else if(piece == ' ') {
                LCD.Write("ERROR: INVALIDPIECEYOU'REDUMB");
            }
            else {
                resetLegalState();
                LCD.Write(piece+" HOWDIDTHISGETHERE");
            }
            //check for illegal moves due to check
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8; j++) {
                    if(legalState[i][j]) {
                        //makes a simulation and makes the move in question and checks if the king is subsequently in check
                        if(!simulateForCheck(file,rank,i,j, isupper(piece))) {
                            legalState[i][j] = false;
                        }
                    }
                }
            }
        }
        //sets the legalState to all falses - to prevent carryover values
        //not called every time since the possible moves of multiple pieces at once may be needed
        void resetLegalState() {
            for(int i = 0; i<8;i++) {
                for(int j = 0; j<8; j++) {
                    legalState[i][j] = false;
                }
            }
        }
        //resets attackedState
        void resetAttackedState() {
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8; j++) {
                    attackedState[i][j] = false;
                }
            }
        }

        //clears what white pawns are en passantable at the end of turn
        void clearWhiteEnPassantable() {
            for(int i = 0; i <8; i++) {
                for(int j = 0; j < 8; j++) {
                    if(boardState[i][j]=='P') {
                        //after black's turn - no white pawns should be en-passantable
                        enPassantable[i][j] = false;
                    }
                }
            }
        }
        //clears black pawns en passantable
        void clearBlackEnPassantable() {
            for(int i = 0; i <8; i++) {
                for(int j = 0; j < 8; j++) {
                    if(boardState[i][j]=='p') {
                        enPassantable[i][j] = false;
                    }
                }
            }
        }

        //gives position of white king, used to determine check/checkmate
        void whiteKing(int*file,int*rank) {
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8;j++) {
                    if(boardState[i][j] == 'K') {
                        //store the found position of king in file and rank
                        *file = i;
                        *rank = j;
                        return;
                    }
                }
            }
        }
        //finds position of black king
        void blackKing(int*file,int*rank) {
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8;j++) {
                    if(boardState[i][j] == 'k') {
                        //store found position of king in file and rank
                        *file = i;
                        *rank = j;
                        return;
                    }
                }
            }
        }
        
        //finds out if making the piece on (f1,r1) move to (f2,r2) would lead to check if it's isWhite for white and !isWhite for black's turn
        //returns false if move is illegal, true if it is fine
        bool simulateForCheck(int f1, int r1, int f2, int r2,bool isWhite) {
            //create a new board with the same position as this one
            board simulate(this -> boardState);
            //make the move in question
            simulate.makeMove(f1,r1,f2,r2);
            //find squares being attacked by enemy
            simulate.findEnemyAttackedSquares(isWhite);
            //move has been made already, so if I'm in check, it should be illegal
            if(isWhite) {
                //if white is in check after white made a move - it's illegal, so return false
                if(simulate.isWhiteInCheck()) {
                    return false;
                }
                //if white isn't in check after making a move - it's legal - return true
                else {
                    return true;
                }
            }
            //same logic as above but for black
            else {
                if(simulate.isBlackInCheck()) {
                    return false;
                }
                else {
                    return true;
                }
            }
        }
        //requires attackedState is already filled in using findEnemyAttackedSquares
        //returns true if white's king is under attack
        bool isWhiteInCheck() {
            int kingFile,kingRank;
            //find location of white king
            whiteKing(&kingFile,&kingRank);
            //if the location of king is under attack by enemy - white is in check
            if (attackedState[kingFile][kingRank]) {
                return true;
            }
            else {
                return false;
            }     
        }
        //same as above but for black
        bool isBlackInCheck() {
            int kingFile,kingRank;
            blackKing(&kingFile,&kingRank);
            if (attackedState[kingFile][kingRank]) {
                return true;
            }
            else {
                return false;
            }
        }
        //call before altering legalState
        //sets allLegalSquares to the union of all (isWhite) pieces' legalStates - if one piece can move there- it's true
        void fillAllLegalMoves(bool isWhite, bool allLegalSquares[8][8]) {
            for (int a = 0; a < 8; a++) {
                for (int b = 0; b < 8; b++) {
                    //initialize allLegalSquares to prevent carrying over values
                    allLegalSquares[a][b] = false;
                }
            }
            //search through the whole board
            for(int i = 0; i<8;i++) {
                for(int j = 0; j<8; j++) {
                    if(isWhite) {
                        if(isupper(boardState[i][j])) {
                            //set legalState to everywhere the piece can move
                            findLegalMoves(i,j);
                            for(int k = 0; k<8; k++) {
                                for(int l = 0; l<8; l++) {
                                    //at each square on board - update allLegalSquares to include the piece we just searched
                                    allLegalSquares[k][l] = legalState[k][l] || allLegalSquares[k][l];
                                }
                            }
                            //reset the legal state to prevent carrying over
                            resetLegalState();
                        }
                    }
                    //same logic as above but for black
                    else {
                        if(islower(boardState[i][j])) {
                            findLegalMoves(i,j);
                            for(int k = 0; k<8; k++) {
                                for(int l = 0; l<8; l++) {
                                    allLegalSquares[k][l] = legalState[k][l] || allLegalSquares[k][l];
                                }
                            }
                            resetLegalState();
                        }
                    }
                }
            }
            //unnecessary but just in case
            resetLegalState();
        }
        //check all conditions for castling and returns true if (isWhite) can castle kingside
        bool canCastleKSide(bool isWhite) {
            int kingPos[2];
            bool canCastle = true;
            if(isWhite) {
                whiteKing(&kingPos[0],&kingPos[1]);
                //check if either piece moved
                if(whiteKingMoved || whiteKingRookMoved) {
                    canCastle = false;
                }
                //check if king is under attack and if the rook is not on current square
                else if(attackedState[kingPos[0]][kingPos[1]]|| boardState[7][7]!= 'R') {
                    canCastle = false;
                }
                else if(!whiteKingMoved && !whiteKingRookMoved) {
                    //check the two squares to the right of the king to see if they are under attack
                    for(int file = kingPos[0]+1; file <= kingPos[0]+2; file++) {
                        //check if the squares are under attack or if the square isn't empty
                        if(attackedState[file][kingPos[1]] || boardState[file][kingPos[1]] != ' ') {
                            canCastle = false;
                        }
                    }
                }
                else {
                    canCastle = false;
                }
            }
            else {
                //same logic as above but for black side
                blackKing(&kingPos[0],&kingPos[1]);
                if(blackKingMoved || blackKingRookMoved) {
                    canCastle = false;
                }
                else if(attackedState[kingPos[0]][kingPos[1]]|| boardState[7][0]!= 'r') {
                    canCastle = false;
                }
                else if(!blackKingMoved && !blackKingRookMoved) {
                    for(int file = kingPos[0]+1; file <= kingPos[0]+2; file++) {
                        if(attackedState[file][kingPos[1]] || boardState[file][kingPos[1]] != ' ') {
                            canCastle = false;
                        }
                    }
                }
                else {
                    canCastle = false;
                }
            }
            return canCastle;
        }
        //same as canCastleKSide except in the other direction
        bool canCastleQSide(bool isWhite) {
            int kingPos[2];
            bool canCastle = true;
            if(isWhite) {
                whiteKing(&kingPos[0],&kingPos[1]);
                if(whiteKingMoved || whiteQueenRookMoved) {
                    canCastle = false;
                }
                else if(attackedState[kingPos[0]][kingPos[1]] || boardState[0][7]!= 'R') {
                    canCastle = false;
                }
                else if(!whiteKingMoved && !whiteQueenRookMoved) {
                    //check squares left of king
                    for(int file = kingPos[0]-1; file >= kingPos[0]-2; file--) {
                        //check if square is under attack or not empty
                        if(attackedState[file][kingPos[1]] || boardState[file][kingPos[1]] != ' ') {
                            canCastle = false;
                        }
                    }
                    //queenside castling needs to check all squares between king and rook, however, b1/b8 must be empty but doesn't matter if it's under attack
                    if(boardState[kingPos[0]-3][kingPos[1]]!= ' ') {
                        canCastle = false;
                    }
                }
                else {
                    canCastle = false;
                }
            }
            else {
                //same logic as above but for black
                blackKing(&kingPos[0],&kingPos[1]);
                if(whiteKingMoved || whiteQueenRookMoved) {
                    canCastle = false;
                }
                else if(attackedState[kingPos[0]][kingPos[1]] || boardState[0][0]!= 'r') {
                    canCastle = false;
                }
                else if(!blackKingMoved && !blackQueenRookMoved) {
                    for(int file = kingPos[0]-1; file >= kingPos[0]-2; file--) {
                        if(attackedState[file][kingPos[1]] || boardState[file][kingPos[1]] != ' ') {
                            canCastle = false;
                        }
                    }
                    if(boardState[kingPos[0]-3][kingPos[1]]!= ' ') {
                        canCastle = false;
                    }
                }
                else {
                    canCastle = false;
                }
            }
            return canCastle;
        }
        //takes a board finds the union of their castling checks
        void copyCastlingChecks(board change) {
            whiteKingMoved=change.whiteKingMoved || whiteKingMoved;
            blackKingMoved = change.blackKingMoved || blackKingMoved;
            blackKingRookMoved = change.blackKingRookMoved || blackKingRookMoved;
            whiteKingRookMoved = change.whiteKingRookMoved || whiteKingRookMoved;
            whiteQueenRookMoved = change.whiteQueenRookMoved || whiteQueenRookMoved;
            blackQueenRookMoved = change.blackQueenRookMoved || blackQueenRookMoved;
        }
        //finds if a single value in legalState is true
        bool legalStateTrue() {
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8; j++) {
                    if(legalState[i][j]) {
                        return true;
                    }
                }
            }
            return false;
        }
        //looks for a random black move and sets the location and destination to parameters
        void makeRandomBlackMove(int*initfile,int*initrank,int*finalfile,int*finalrank) {
            //sets simulate to this
            board simulate = *this;
            //store all piece positions
            int positions [16][2];
            //count number of pieces black has
            int count = 0;
            //reset legal state and find white's attacked squares
            simulate.resetLegalState();
            simulate.findEnemyAttackedSquares(false);
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8; j++) {
                    if(islower(simulate.boardState[i][j])) {
                        //find legal moves for each piece
                        simulate.findLegalMoves(i,j);
                        //if legal moves exist- add the position to positions and increment count
                        if(simulate.legalStateTrue()) {
                            positions[count][0] = i;
                            positions[count][1] = j;
                            count++;
                        }
                        //reset the legal state to prevent carry over
                        simulate.resetLegalState();
                    }
                }
            }
            //choose a random number between 0 and count-1
            int random = randomInRange(0,count);
            //set piecePosition to the location of the random piece that was chosen
            int piecePosition [2] = {positions[random][0],positions[random][1]};
            //set initfile and initrank to the position of the piece chosen
            *initfile = piecePosition[0];
            *initrank = piecePosition[1];
            //reset the legal state
            simulate.resetLegalState();
            //find the legal moves of the chosen piece
            simulate.findLegalMoves(piecePosition[0],piecePosition[1]);
            int moves [64][2];
            int numMoves = 0;
            //add all legal move positions to moves
            //numMoves will be the number of moves that piece has
            for(int i = 0; i<8; i++) {
                for(int j = 0; j<8;j++) {
                    if(simulate.legalState[i][j]) {
                        moves[numMoves][0] = i;
                        moves[numMoves][1] = j;
                        numMoves++;
                    }
                }
            }
            //choose a new random number
            int random2 = randomInRange(0,numMoves);
            //set the final location to the respective position in moves
            *finalfile = moves[random2][0];
            *finalrank = moves[random2][1];            
        }
};

//opens all sprites
FEHImage pawn_bl ("art/pawn_bl.png");
FEHImage pawn_wh ("art/pawn_wh.png");
FEHImage rook_bl ("art/rook_bl.png");
FEHImage rook_wh ("art/rook_wh.png");
FEHImage bishop_bl ("art/bishop_bl.png");
FEHImage bishop_wh ("art/bishop_wh.png");
FEHImage knight_bl ("art/knight_bl.png");
FEHImage knight_wh ("art/knight_wh.png");
FEHImage queen_bl ("art/queen_bl.png");
FEHImage queen_wh ("art/queen_wh.png");
FEHImage king_bl ("art/king_bl.png");
FEHImage king_wh ("art/king_wh.png");
FEHImage menuimage ("art/menu.png");



/*
*******************
*******************
FUNCTION PROTOTYPES
*******************
*******************
*/


//MENU FUNCTIONS
void start(); //starts game menu
void drawMenu(); //draws menu
void menuChoice(); //figues out what was clicked on the menu
void locateTouch(float *x,float *y); //locates touch using LCD.Touch
void locateTouch(float*x,float*y, timer* time); //takes time off of the timer while locating touch
bool coordsInBox(float,float, float,float,float,float); //checks if given coordinates are in given box, returns true/false
void openGameOptions(); // clears screen and goes to game options screen
void openStats();// clears screen and opens stats
void openInstructions(); //clears screen and goes to instructions
void openCredits(); //clears screen and goes to credits
void createMenuButton(); //creates button at bottom that goes back to the menu
void backTouched(); // waits until the aforementioned back button is clicked

//GAME OPTIONS
void drawGameOptions(); //draws game options menu
void gameOptionsChoice(); //allows user to choose option off game options menu
void createExitButton(); //creates button to exit game
void ifExitGame(float x, float y); //checks if game exit button has been pressed and sends game back to main menu


//GAME FUNCTIONS
void startGame(); //starts game, game will run within startGame loop
void startGameVsComputer(); //starts game where black automatically plays moves
void drawBoard(); //draws blank board
void drawCurrentPosition(char[8][8]); //draws pieces in current position given by boardState
void drawCurrentPosition(char[8][8], timer* p1, timer* p2);
void moveAnimate(board* temp, char piece, int initfile, int initrank, int finalfile, int finalrank, timer*p1,timer*p2); //animates piece movement
bool gameOver(float* result, board temp, bool isWhite); //checks if game is over
void colorSquare(int, int); //highlights square of piece when clicked
void findSquare(float x, float y, int*file, int*rank); //finds which square was clicked and stores to file and rank pointers
void clearSquare(int,int); //clears square highlight when selected square is changed
float move(board* temp, timer* p1, timer* p2);
void highlightChosenSquares(bool[8][8]); //highlights legal moves
void clearChosenSquares(bool[8][8]); //clears legal moves
FEHImage convertCharToImage(char c); //converts chars (used to store game state) to sprites (used to display game)
void TimerOverProtocol(timer* time); //ends game if a timer expires

int findXImageOffset(char c);
int findYImageOffset(char c);




/*
Main: starts game
*/

int main()
{

    start ();
    while (1) {
        LCD.Update();
        // Never end
    }
    return 0;
}







/*
********************
********************
FUNCTION DEFINITIONS
********************
********************
*/


/*
locates touch and stores to input variables, one with timer and one without
*/

void locateTouch(float* x,float* y) //no timer, for use in menus
{
    while (!LCD.Touch(x, y));
        //wait for touch
    
    while (LCD.Touch(x, y));
        //locate touch
    //after function end, x and y contain loactions of click
}

void locateTouch(float* x,float* y, timer* time) //with timer, for use in game
{
    while (!LCD.Touch(x, y)) {
        //wait for touch
        //run timer while waiting
        time->passTwentiethSecond();
        time -> displayTimer();
        if(time -> timeOver()) {
            TimerOverProtocol(time);
        }
    }
    
    while (LCD.Touch(x, y)) {
        //locate touch
        //continue to run/update timer while touch is happening
        time->passTwentiethSecond();
        time -> displayTimer();
        if(time -> timeOver()) {
            TimerOverProtocol(time);
        }
    }
    //after function end, x and y contain loactions of click
}


/*
start game (go to main menu)
*/

void start()
{
    //draw the menu
    drawMenu();
    //wait for/determine choice, open chosen menu item
    menuChoice();
}





/*
**************
MENU FUNCTIONS
**************
*/




/*
Draws main menu
*/

void drawMenu() 
{
    LCD.Clear();
    //background
    LCD.SetBackgroundColor (BLACK);

    //game title
    LCD.SetFontScale (2.0);
    LCD.SetFontColor (SCARLET);
    LCD.WriteAt ("Chess", 70, 5);
    menuimage.Draw (210, 5);
    
    LCD.SetFontColor(WHITE);
    LCD.SetFontScale(1.0);

    //play
    LCD.DrawRectangle(XMAX/2-100, YMAX-190, MenuWidth,MenuHeight);
    LCD.WriteAt("Play Game", XMAX/2-55, YMAX-185);

    //statistics
    LCD.DrawRectangle(XMAX/2-100, YMAX-150, MenuWidth,MenuHeight);
    LCD.WriteAt("View Statistics", XMAX/2-90, YMAX-145);

    //instructions
    LCD.DrawRectangle(XMAX/2-100, YMAX-110, MenuWidth,MenuHeight);
    LCD.WriteAt("Instructions", XMAX/2-75, YMAX-105);

    //credits
    LCD.DrawRectangle(XMAX/2-100, YMAX-70, MenuWidth,MenuHeight);
    LCD.WriteAt("Credits", XMAX/2-40, YMAX-65);

    //lol
    LCD.SetFontScale(0.5);
    LCD.WriteAt("Ignore your professor like never before!", XMAX/2-120, YMAX-25);
    LCD.SetFontColor(GRAY);
    LCD.WriteAt("*We do not endorse this behavior", XMAX/2-100, YMAX-10);
    LCD.SetFontColor(WHITE);

}


/*
finds menu choice and goes to selection
*/

void menuChoice()
{
    //bool to track if something has been chosen
    bool somethingChosen = false;
    //so store position
    float x,y;

    //loop until something is chosen
    while(!somethingChosen) {
        
        //wait for new coodinates to touch
        locateTouch (&x,&y);

        //use coordsInBox function to check if touch was in box
        if (coordsInBox(x,y,XMAX/2-100,YMAX-190,MenuWidth,MenuHeight) == true)
        {
            //Play Game box clicked
            somethingChosen = true;
            openGameOptions();
        }
        else if (coordsInBox(x,y,XMAX/2-100,YMAX-150,MenuWidth,MenuHeight) == true)
        {
            //Statistics clicked
            somethingChosen = true;
            openStats();
        }
        else if (coordsInBox(x,y,XMAX/2-100,YMAX-110,MenuWidth,MenuHeight) == true)
        {
            //Instructions clicked
            somethingChosen = true;
            openInstructions();
        }
        else if (coordsInBox(x,y,XMAX/2-100,YMAX-70,MenuWidth,MenuHeight) == true)
        {
            //Credits clicked
            somethingChosen = true;
            openCredits();
        }
        
    }
}

/*
Checks if given cooridinates are within a box, used in menus and for exit button
*/

bool coordsInBox(float x, float y, float leftX, float leftY, float width, float height) {
    //is x within the width of box and y within height of box?
    return (x <= leftX+width && x>= leftX) && (y>=leftY && y <= leftY+height);
}

/*
following functions open stats, instructions, and credits
*/

//opens statistics page, calls return to menu button function
void openStats()
{
    LCD.Clear();
    LCD.SetFontScale (1.5);
    LCD.WriteAt("Statistics:", 0, 0);
    LCD.SetFontScale (0.5);
    LCD.WriteAt("Moves Made:", 0, 35);
    LCD.WriteAt(totalmoves, 100, 35);
    LCD.WriteAt("Games played:", 0, 60);
    LCD.WriteAt(gamesplayed, 100, 60);
    LCD.WriteAt("Seconds played:", 0, 85);
    LCD.WriteAt((int)timeplayed, 100, 85);
    LCD.WriteAt("Pieces Captured:", 0, 110);
    LCD.WriteAt(piecescaptured, 100, 110);
    createMenuButton();
}

//opens instructions page, calls return to menu button function
void openInstructions()
{
    LCD.Clear();
    LCD.SetFontScale(0.5);
    LCD.WriteAt ("There are two sides. White goes first.", 5, 5);
    LCD.WriteAt ("Click on a piece to see available moves.", 5, 25);
    LCD.WriteAt ("Clicking on an available move will move the piece", 5, 45);
    LCD.WriteAt ("to that location.", 5, 55);
    LCD.WriteAt ("The game ends when one of the players is put into", 5, 75);
    LCD.WriteAt ("checkmate.", 5, 85);
    LCD.WriteAt ("Checkmate is when a player cannot make any move to", 5, 105);
    LCD.WriteAt ("prevent thier king from being taken.", 5, 115);
    LCD.WriteAt ("Enjoy!", 5, 135);
    createMenuButton();
}

//opens credits page, calls return to menu button function
void openCredits()
{
    LCD.Clear();
    LCD.SetFontScale (1.5);
    LCD.WriteAt("Credits", 0, 0);
    LCD.SetFontScale (1.0);
    LCD.WriteAt("Matt Wang",  70, 35);
    LCD.WriteAt("Owen Goodwin",  70, 60);
    createMenuButton();
}

/*
Return to menu botton
*/

//creates button
void createMenuButton() {
    LCD.SetFontScale(0.5);
    LCD.DrawRectangle(XMAX/2+50, YMAX-70, MenuWidth-90,MenuHeight-10);
    LCD.WriteAt("Return to Menu", XMAX/2+60, YMAX-65);
    backTouched();
}

//checks if return to menu botton was pressed
void backTouched() {
    bool clicked = false;
    float x,y;
    while(!clicked) {
        locateTouch(&x,&y);
        if (coordsInBox(x,y,XMAX/2+50,YMAX-70,MenuWidth-90,MenuHeight-10) == true)
        {
            //back clicked
            clicked = true;
            start();
        }
    }
}

/*
Game options menu
*/

//goes to game options menu
void openGameOptions() //
{
    LCD.Clear();
    drawGameOptions();
    gameOptionsChoice();
}

//draws game options menu
void drawGameOptions() {
    LCD.SetFontScale(1.0);

    //play local multiplayer
    LCD.DrawRectangle(XMAX/2-100, YMAX-190, MenuWidth,MenuHeight);
    LCD.WriteAt("Local PvP", XMAX/2-50, YMAX-185);
    //play vs computer
    LCD.DrawRectangle(XMAX/2-100, YMAX-150, MenuWidth,MenuHeight);
    LCD.WriteAt("Vs. Computer", XMAX/2-70, YMAX-145);
    //back
    LCD.DrawRectangle(XMAX/2-100, YMAX-110, MenuWidth,MenuHeight);
    LCD.WriteAt("Back", XMAX/2-20, YMAX-105);
}

//finds what is selected on game options 
void gameOptionsChoice() {
    //bool to track if a

        bool somethingChosen = false;
    //position
    float x,y;

    //loop until something is chosen
    while(!somethingChosen) {
        //wait for new coodinates to touch
        locateTouch (&x,&y);
        //use function to check if touch was in box
        if (coordsInBox(x,y,XMAX/2-100,YMAX-190,MenuWidth,MenuHeight) == true)
        {
            //local mult clicked
            somethingChosen = true;
            startGame();
        }
        else if (coordsInBox(x,y,XMAX/2-100,YMAX-150,MenuWidth,MenuHeight) == true)
        {
            //vs. computer clicked
            somethingChosen = true;
            startGameVsComputer();
        }
        else if (coordsInBox(x,y,XMAX/2-100, YMAX-110, MenuWidth,MenuHeight) == true)
        {
            //back clicked
            somethingChosen = true;
            start();
        }
    }
}





/*
**************
GAME FUNCTIONS
**************
*/



/*
starts game, game runs within this function, using other following functions
*/

void startGame () {
    //increase games played stat tracker
    gamesplayed++;

    //draw game board
    //drawBoard();

    board temp;
    timer p1('w',false);
    timer p2('b',false);
    float result=-2.0;
    p1.setActivated(); //sets original result state (none)
    p2.setActivated();
    temp.fillStartingPieces();
    drawCurrentPosition(temp.boardState,&p1,&p2);

    //play game     
    while(result <-1.0) {
        result = move(&temp, &p1,&p2);//move function found below, tracks and execturiutems moves while in game    
    }
    LCD.Clear();
    drawCurrentPosition(temp.boardState);


    //prints game result after game over (game over menu)
    LCD.SetFontColor(WHITE);
    LCD.FillRectangle(XMAX/2 - END_SCREEN_LENGTH/2-TOP_LEFT_BOARD_X, YMAX/2 - END_SCREEN_LENGTH/2, END_SCREEN_LENGTH,END_SCREEN_LENGTH);
    LCD.SetFontColor(BLACK);

    if(result <0) { //black victory
        
        LCD.WriteAt("Black Wins!", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    else if(result <1) { //draw
        LCD.WriteAt("Draw.", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    else { //white victory
        LCD.WriteAt("White Wins!", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }

    createExitButton();
    float x,y;
    locateTouch(&x,&y);
    ifExitGame(x,y);
}

/*
exact same functionality as startGame, just with computer as player 2
*/

void startGameVsComputer() {
    gamesplayed++;
    //drawBoard();
    board temp;
    timer p1('w',false);
    timer p2('b',true);
    float result=-2.0;
    p1.setActivated(); //sets original result state (none)
    p2.setActivated();
    temp.fillStartingPieces();
    drawCurrentPosition(temp.boardState,&p1,&p2);

    //play game     
    while(result <-1.0) {
        result = move(&temp, &p1,&p2);//move function found below, tracks and execturiutems moves while in game    
        //LCD.Write(result);
    }
    LCD.Clear();
    drawCurrentPosition(temp.boardState);
    //prints game result after game over (game over menu)
    LCD.SetFontColor(WHITE);
    LCD.FillRectangle(XMAX/2 - END_SCREEN_LENGTH/2-TOP_LEFT_BOARD_X, YMAX/2 - END_SCREEN_LENGTH/2, END_SCREEN_LENGTH,END_SCREEN_LENGTH);
    LCD.SetFontColor(BLACK);

    if(result <0) {
        LCD.WriteAt("Black Wins!", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    else if(result <1) {
        LCD.WriteAt("Draw.", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    else {
        LCD.WriteAt("White Wins!", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }

    createExitButton();
    float x,y;
    locateTouch(&x,&y);
    ifExitGame(x,y);
}








/*
*************************************************************
move funtion tracks and implements moves made during gameplay
*************************************************************
*/




float move(board* temp, timer *p1,timer*p2) {

    float x,y; //location variables (touch) 
    int FromFile = -1, FromRank = -1, ToFile = -1, ToRank = -1; //selected squares, initalize to -1, which is a state that will never be reached in game
    float result = -2.0; //no result 
    bool over = false; //game not over
    
    /*
    *****White movement*****
    */

    //find what squares enemy is attacking
    temp -> resetAttackedState();
    temp -> findEnemyAttackedSquares(true); //used to determine legal moves (to prevent checkmate)
    temp -> resetLegalState();
    over = gameOver(&result, *temp, true);

    //if(over) {return result;}
    //else {return -2;}

    if(temp -> isWhiteInCheck()) {
        //print if white is in check    
        LCD.WriteAt("CHECK",5,5);
    }

    //will only enter while loop if square has not been selected, will repeat until movable piece has been selected  
    while(FromFile < 0 || FromRank <0 || ToFile <0 || ToRank <0) {
        //set to location to -1 since a from hasn't been chosen yet
        ToFile = -1; ToRank = -1;
        //wait for player to touch a square until an interesting one is clicked
        while(FromFile < 0 || FromRank < 0) {
            //wait for touch while also changing the timer
            locateTouch(&x,&y, p1);
            //detect if exit was clicked
            ifExitGame(x, y);
            //find the square that was clicked
            findSquare(x,y,&FromFile,&FromRank);
            //if the square clicked is invalid or isn't a white piece - set back to -1 
            if(FromFile <0 || FromRank<0 || !isupper(temp -> boardState[FromFile][FromRank])) {
                    FromFile = -1; FromRank = -1; //set no destination square
            } 
            //valid square was chosen
            else {
                //highlight the square
                colorSquare(FromFile,FromRank);
                //redraw the piece that was just drawn over
                FEHImage spriteAt = convertCharToImage(temp->boardState[FromFile][FromRank]);
                spriteAt.Draw(TOP_LEFT_BOARD_X+FromFile*SQUARE_LENGTH + findXImageOffset(temp->boardState[FromFile][FromRank]), TOP_LEFT_BOARD_Y + FromRank*SQUARE_LENGTH + findYImageOffset(temp->boardState[FromFile][FromRank]));
                //reset the legal state
                temp -> resetLegalState();
                //find legal moves for that piece
                temp->findLegalMoves(FromFile, FromRank);
                //highligh the legal moves for the piece
                highlightChosenSquares(temp->legalState);
            }
        }
        //while no destination chosen
        while(ToFile < 0 || ToRank <0) {
            //finds where a player clicks and decreases timer
            locateTouch(&x, &y,p1);
            //detect if player clicked exit game
            ifExitGame(x, y);
            //clear the square that was highlighted earlier
            clearSquare(FromFile, FromRank);
            //clear the other squares that were highlighted
            clearChosenSquares(temp->legalState);
            //redraw the pieces that were likely cleared by the previous 2 lines
            drawCurrentPosition(temp->boardState, p1,p2);
            //find the square
            findSquare(x,y,&ToFile, &ToRank);
            //if the square is valid
            if(ToFile>=0 && ToRank >=0) {
                //if another white piece is clicked - need to set that piece as the new piece to move
                if(isupper(temp->boardState[ToFile][ToRank])) {
                    //set the from variables to the to variables
                    FromFile = ToFile; FromRank = ToRank;
                    //clear the squares
                    clearChosenSquares(temp->legalState);
                    //redraw
                    drawCurrentPosition(temp->boardState,p1,p2);
                    //reset legal state
                    temp->resetLegalState();
                    //find the legal moves of the newly chosen piece
                    temp->findLegalMoves(FromFile, FromRank);
                    //highlight all of the relevant squares
                    highlightChosenSquares(temp->legalState);
                    colorSquare(ToFile,ToRank);
                    //redraw piece
                    FEHImage spriteAt = convertCharToImage(temp->boardState[ToFile][ToRank]);
                    spriteAt.Draw(TOP_LEFT_BOARD_X+FromFile*SQUARE_LENGTH + findXImageOffset(temp->boardState[FromFile][FromRank]), TOP_LEFT_BOARD_Y + FromRank*SQUARE_LENGTH + findYImageOffset(temp->boardState[FromFile][FromRank]));
                    //set to variables to -1 since no destination has been chosen
                    ToFile = -1; ToRank = -1;
                }
                else {
                    //if the square is valid but illegal
                    if(!(temp->legalState[ToFile][ToRank])) {
                        //set from variables to -1 as a new piece needs to be chosen - will loop back to beginning of outer loop
                        FromFile = -1; FromRank = -1;
                        drawCurrentPosition(temp -> boardState,p1,p2);
                    }
                    //if the piece is a pawn - stuff needs to be checked
                    else if(temp -> boardState[FromFile][FromRank] == 'P') {
                        if(ToRank == FromRank - 2) {
                            //if a pawn theoretically moved here - it would be en passantable - so set it so
                            temp -> enPassantable[ToFile][ToRank] = true;
                        }
                        else {
                            //if a pawn moved, the square it moved from is guaranteed not enpassantable
                            temp -> enPassantable[FromFile][FromRank] = false;
                        }
                        if(ToRank == FromRank -1 && ToFile == FromFile -1 && temp ->boardState[ToFile][ToRank] == ' ') {
                            //if the pawn took en passant to the left - capture the piece that just got captured and set the enpassantable to false since it's dead
                            temp -> boardState[FromFile-1][FromRank] = ' ';
                            temp -> enPassantable[FromFile-1][FromRank] = false;
                        }
                        //same as previous if but in the other horizontal direction
                        else if(ToRank == FromRank -1 && ToFile == FromFile +1 && temp ->boardState[ToFile][ToRank] == ' ') {
                            temp -> boardState[FromFile+1][FromRank] = ' ';
                            temp -> enPassantable[FromFile+1][FromRank] = false;
                        }
                    }
                }
            }
        }

    }
    
    //by now, white has chosen a piece to move and a valid square to move it to

    //animate movement
    moveAnimate(temp, temp->boardState[FromFile][FromRank], FromFile, FromRank, ToFile, ToRank,p1,p2);
    drawCurrentPosition(temp->boardState,p1,p2);

    //add 1 to moves made counter
    totalmoves++;

    //white just made move - check if game is over now
    over = gameOver(&result, *temp, false);

    //return result if game is over
    if(over) {return result;}

    


    /*
    *****Black movement*****
    */


    //reset the attacked state
    temp -> resetAttackedState();

    //enter 'if' as long as player 2 is human
    if(!(p2 -> isComputer)) {
        //find squares white attacks
        temp -> findEnemyAttackedSquares(false);
        //reset the legal state
        temp->resetLegalState();
        //after white moves - all of black's pawns are guarenteed not en passantable
        temp -> clearBlackEnPassantable();
        //print check message if black is in check
        if(temp -> isBlackInCheck()) {
            LCD.WriteAt("CHECK",5,5);
        }
        //same process as for white
        FromFile = -1; FromRank = -1; ToFile = -1; ToRank = -1;
        //go until player clicks valid black piece
        while(FromFile < 0 || FromRank <0 || ToFile <0 || ToRank <0) {
            ToFile = -1; ToRank = -1;
            while(FromFile < 0 || FromRank < 0) {
                locateTouch(&x,&y, p2);
                ifExitGame(x, y);
                findSquare(x,y,&FromFile,&FromRank);
                if(FromFile <0 || FromRank<0 || !islower(temp -> boardState[FromFile][FromRank])) {
                    FromFile = -1; FromRank = -1;
                }
                else {
                    colorSquare(FromFile,FromRank);
                    FEHImage spriteAt = convertCharToImage(temp->boardState[FromFile][FromRank]);
                    spriteAt.Draw(TOP_LEFT_BOARD_X+FromFile*SQUARE_LENGTH + findXImageOffset(temp->boardState[FromFile][FromRank]), TOP_LEFT_BOARD_Y + FromRank*SQUARE_LENGTH + findYImageOffset(temp->boardState[FromFile][FromRank]));
                    /*LCD.SetFontColor(RED);
                    LCD.WriteAt(temp->boardState[FromFile][FromRank], TOP_LEFT_BOARD_X+FromFile*SQUARE_LENGTH, TOP_LEFT_BOARD_Y+FromRank*SQUARE_LENGTH); */
                    temp -> resetLegalState();
                    temp->findLegalMoves(FromFile, FromRank);
                    highlightChosenSquares(temp->legalState);
                }
            }
            //go until player clicks anywhere
            //if player clicks on valid square - set the to variables to those coordinates and leave the loop
            //if player clicks on another of their own pieces, set the from variables to the clicked coordinates and loop back
            //if player clicks anywhere else - set from variables back to -1 and loop back to the first loop
            while(ToFile < 0 || ToRank <0) {
                locateTouch(&x, &y,p2);
                ifExitGame(x, y);
                drawCurrentPosition(temp->boardState,p1,p2);
                findSquare(x,y,&ToFile, &ToRank);
                if(ToFile >=0 && ToRank >=0) {
                    clearSquare(FromFile, FromRank);
                    clearChosenSquares(temp->legalState);
                    if(islower(temp->boardState[ToFile][ToRank])) {
                        FromFile = ToFile; FromRank = ToRank;
                        clearChosenSquares(temp->legalState);
                        drawCurrentPosition(temp->boardState,p1,p2);
                        temp->resetLegalState();
                        temp->findLegalMoves(FromFile, FromRank);
                        highlightChosenSquares(temp->legalState);
                        colorSquare(ToFile,ToRank);
                        FEHImage spriteAt = convertCharToImage(temp->boardState[FromFile][FromRank]);
                        spriteAt.Draw(TOP_LEFT_BOARD_X+FromFile*SQUARE_LENGTH + findXImageOffset(temp->boardState[FromFile][FromRank]), TOP_LEFT_BOARD_Y + FromRank*SQUARE_LENGTH + findYImageOffset(temp->boardState[FromFile][FromRank]));
                        ToFile = -1; ToRank = -1;
                    }
                    else {
                        if(!(temp->legalState[ToFile][ToRank])) {
                            FromFile = -1; FromRank = -1;
                            drawCurrentPosition(temp -> boardState,p1,p2);
                        }
                        //same en passant detection and storage as for white
                        else if(temp -> boardState[FromFile][FromRank] == 'p') {
                            if(ToRank == FromRank + 2) {
                                temp -> enPassantable[ToFile][ToRank] = true;
                            }
                            else {
                                temp -> enPassantable[FromFile][FromRank] = false;
                            }
                            if(ToRank == FromRank +1 && ToFile == FromFile -1 && temp ->boardState[ToFile][ToRank] == ' ') {
                                temp -> boardState[FromFile-1][FromRank] = ' ';
                                temp -> enPassantable[FromFile-1][FromRank] = false;
                            }
                            else if(ToRank == FromRank +1 && ToFile == FromFile +1 && temp ->boardState[ToFile][ToRank] == ' ') {
                                temp -> boardState[FromFile+1][FromRank] = ' ';
                                temp -> enPassantable[FromFile+1][FromRank] = false;
                            }
                        }
                    }
                }
            }
            //clear white's en passantable as black has just moved
            temp -> clearWhiteEnPassantable();
        }
        //by now, black has chosen a piece to move and a valid square to move it to
        //animate the move and update the boardState accordingly
        moveAnimate(temp, temp->boardState[FromFile][FromRank], FromFile, FromRank, ToFile, ToRank,p1,p2);
        //redraw position
        drawCurrentPosition(temp->boardState,p1,p2);
    }

    //if player 2 is computer, execute 'else'
    else {
        int initfile = -1, initrank = -1, finalfile = -1,finalrank = -1;
        //find white's attacked squares
        temp -> findEnemyAttackedSquares(false);
        //reset legal state
        temp->resetLegalState();
        //reset black en passantable as white has just moved
        temp -> clearBlackEnPassantable();
        //display if black is in check
        if(temp -> isBlackInCheck()) {
            LCD.WriteAt("CHECK",5,5);
        }
        //set initfile,initrank, finalfile, and finalrank to the coordinate of a random piece and a random square that piece can move to
        temp->makeRandomBlackMove(&initfile,&initrank,&finalfile,&finalrank);
        //wait 1 second for the illusion of 'thinking'
        Sleep(1.0);
        //decrement the timer for computer to further give the illusion of 'thinking'
        p2->timeLeft--;
        //increment time played
        timeplayed++;
        //animate the move
        moveAnimate(temp,temp -> boardState[initfile][initrank], initfile,initrank,finalfile,finalrank,p1,p2);
        //redraw the position
        drawCurrentPosition(temp->boardState,p1,p2);

    }
    //add 1 to moves made counter
    totalmoves++;
    //black just made move - check for game over
    over = gameOver(&result, *temp, true);
    if(over) {return result;}
    else
    //-2 means the game is still ongoing
    {return -2;}
}





/*
Draws Game Board
*/





void drawBoard() {

    LCD.Clear();
    //draw 8x8 board
    LCD.SetFontColor (BURLYWOOD);
    //draw board skeleton
    LCD.FillRectangle(TOP_LEFT_BOARD_X - BORDERWIDTH, TOP_LEFT_BOARD_Y - BORDERWIDTH, 8*SQUARE_LENGTH + 2*BORDERWIDTH,8*SQUARE_LENGTH + 2*BORDERWIDTH);
    for (int i = 0; i < 8; i++)
    {
        for (int k = 0; k < 8; k++) {
            if((i+k)%2 == 0) {
                LCD.SetFontColor(GRAY);
            }
            else {
                LCD.SetFontColor(SCARLET);
            }
            LCD.FillRectangle(TOP_LEFT_BOARD_X + (i * SQUARE_LENGTH), TOP_LEFT_BOARD_Y + (k * SQUARE_LENGTH), SQUARE_LENGTH, SQUARE_LENGTH);   
        }
    }
}

/*
Following functions display the current postion of the gamestate visually, one with timers and one without
*/

//draws position without timers, used when time is not updating (during animation)
void drawCurrentPosition(char gameState[8][8]) {
    LCD.Clear();
    drawBoard();
    createExitButton();
    LCD.SetFontScale(0.5);
    LCD.SetFontColor(RED);
    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8; j++) {
            if(gameState[i][j] != ' ') {
                FEHImage spriteAt = convertCharToImage(gameState[i][j]);
                spriteAt.Draw((i*SQUARE_LENGTH)+TOP_LEFT_BOARD_X + findXImageOffset(gameState[i][j]),(j*SQUARE_LENGTH)+TOP_LEFT_BOARD_Y +findYImageOffset(gameState[i][j]));
            }
        }
    }
}
//draws position with timers, used when timers are updating
void drawCurrentPosition(char gameState[8][8], timer* p1, timer*p2) {
    LCD.Clear();
    drawBoard();
    createExitButton();
    LCD.SetFontScale(0.5);
    LCD.SetFontColor(RED);
    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8; j++) {
            if(gameState[i][j] != ' ') {
                FEHImage spriteAt = convertCharToImage(gameState[i][j]);
                spriteAt.Draw((i*SQUARE_LENGTH)+TOP_LEFT_BOARD_X + findXImageOffset(gameState[i][j]),(j*SQUARE_LENGTH)+TOP_LEFT_BOARD_Y +findYImageOffset(gameState[i][j]));
            }
        }
    }
    p1 -> displayTimer();
    p2 -> displayTimer();
}


//finds what indices of gameState correspond with position (x,y)
void findSquare(float x, float y, int*file, int*rank) {
    *file = floor((x-TOP_LEFT_BOARD_X)/SQUARE_LENGTH);
    if(*file > 7 || *file <0 )
    {
        *file = -1;
        *rank = -1;
    }
    else {
        *rank = floor((y-TOP_LEFT_BOARD_Y)/SQUARE_LENGTH);
        if(*rank > 7 || *rank < 0) {
            *rank = -1;
            *file = -1;
        }
    }
}

/*
moveAnimate animates the motion of a piece from any square to another
*/


void moveAnimate(board* temp, char piece, int initfile, int initrank, int finalfile, int finalrank, timer*p1,timer*p2) {
    board change; //board object declared just to use boardState variable, will store passed in (initial) gamestate to variable
    char gameState[8][8]; //function game state variable

    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8;j++) {
            gameState[i][j] = temp->boardState[i][j]; //passed in gamestate put into local variable
        }
    }
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            change.boardState[i][j] = gameState[i][j]; //local variable to temporary object variable
        }
    }

    //find what sprite is moving
    FEHImage spriteAt = convertCharToImage(change.boardState[initfile][initrank]);

    //change file and rank of moved piece in temporary variable
    change.makeMove(initfile, initrank, finalfile, finalrank);

    //check for/implement castling if necessary
    if(toupper(gameState[initfile][initrank]) == 'K') {
        //castled kingside
        if(finalfile - initfile == 2) {
            change.makeMove(7,initrank,5,initrank);
        }
        //queenside
        else if(finalfile - initfile == -2) {
            change.makeMove(0,initrank,3,initrank);
        }
    }

    //increase pieces captured stat when the second pair of coordinates is not blank (i.e. moves piece into occupied spot)
    if (gameState[finalfile][finalrank] != ' ')
    {
        piecescaptured++;
    }

    //calculate frame steps
    int framecount = 10;
    float stepsizeY = ((finalrank*SQUARE_LENGTH) - (initrank*SQUARE_LENGTH))/framecount;
    float stepsizeX = ((finalfile*SQUARE_LENGTH) - (initfile*SQUARE_LENGTH))/framecount;

    //image offsets for animation
    float ximageoffset = findXImageOffset(gameState[initfile][initrank]);
    float yimageoffset = findYImageOffset(gameState[initfile][initrank]);
   
    //clear piece from local gamestate (want to print both initial and final locations as blank during animation)
    gameState[initfile][initrank] = ' ';

    //ANIMATION
    //for each frame
    for (int i = 0; i < framecount + 1; i++)
    {
        //clear board, print board, write current gamestate
        drawCurrentPosition(gameState,p1,p2);

        //draw current animated position of piece
        spriteAt.Draw(TOP_LEFT_BOARD_X+initfile*SQUARE_LENGTH + i*stepsizeX + ximageoffset, TOP_LEFT_BOARD_X + initrank*SQUARE_LENGTH+i*stepsizeY + yimageoffset);
        
        //sleep
        Sleep(50);
    }
    
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            temp->boardState[i][j] = change.boardState[i][j]; //assigns real gamestate
        }
    }

    //make sure if a castling piece moves, it is stored in temp
    temp->copyCastlingChecks(change);
}

/*
checks for game over conditions (checkmate)
*/

bool gameOver(float * result, board temp, bool isWhite) {
    bool canMove[8][8];
    for(int i = 0; i<8;i++) {
        for(int j =0; j<8; j++) {
            canMove[i][j] = false;
        }
    }
    bool legalExists = false;
    *result = -2.0;
    if(isWhite) {
        temp.resetLegalState();
        temp.fillAllLegalMoves(true, canMove);
        for(int i = 0; i<8;i++) {
            for(int j = 0; j<8;j++) {
                if(canMove[i][j]) {
                    legalExists = true;
                }
            }
        }
        if(!legalExists) {
            if(temp.isWhiteInCheck()) {
                *result = -1.0;
            }
            else {
                *result = 0.0;
            }
        }
    }
    else {
        temp.resetLegalState();
        temp.fillAllLegalMoves(false, canMove);
        for(int i = 0; i<8;i++) {
            for(int j = 0; j<8;j++) {
                if(canMove[i][j]) {
                    legalExists = true;
                }
            }
        }
        if(!legalExists) {
            if(temp.isBlackInCheck()) {
                *result = 1.0;
            }
            else {
                *result = 0.0;
            }
        }
    }
    return !legalExists;
}

/*
Following functions highlight/clear highling selected piece
*/

//highlights square yellow - intended for highlighting
//if there was a piece on that square- it needs to be redrawn after the function call
void colorSquare(int file, int rank) {
    LCD.SetFontColor(YELLOW);
    LCD.FillRectangle(TOP_LEFT_BOARD_X+ file * SQUARE_LENGTH, TOP_LEFT_BOARD_Y + rank*SQUARE_LENGTH, SQUARE_LENGTH,SQUARE_LENGTH);
}
//empties square - intended for unhighlighting
//if there was a piece on that square- it needs to be redrawn after the function call
void clearSquare(int file, int rank) {
    if((file+rank)%2==0) {
        LCD.SetFontColor(GRAY);
    }
    else {
        LCD.SetFontColor(SCARLET);
    }
    LCD.FillRectangle(TOP_LEFT_BOARD_X+ file * SQUARE_LENGTH, TOP_LEFT_BOARD_Y + rank*SQUARE_LENGTH, SQUARE_LENGTH,SQUARE_LENGTH);


}

/*
Following functions highlight/clear highlight legal moves
*/

//highlights squares that are true on legal - to show legal moves
void highlightChosenSquares(bool legal[8][8]) {
    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8; j++) {
            if(legal[i][j]) { //only true squares
                LCD.SetFontColor(DARKGRAY);
                LCD.FillCircle(TOP_LEFT_BOARD_X+ (i+0.5) * SQUARE_LENGTH, TOP_LEFT_BOARD_Y + (j+0.5)*SQUARE_LENGTH, CIRCLE_R);
            }
        }
    }
}

//clears squares that are true on legal - intended to unhighlight squares
void clearChosenSquares(bool legal[8][8]) {
    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8; j++) {
            if(legal[i][j]) { //only true squares
                clearSquare(i,j);
            }
        }
    }
}

/*
Following functions are used to assist when displaying pieces
*/

//returns whatever image corresponds to the letter (letter used to store gamestate)
FEHImage convertCharToImage(char c) {
    switch (c) {
        case 'p':
            return pawn_bl;
            break;
        case 'P':
            return pawn_wh;
            break;
        case 'r':
            return rook_bl;
            break;
        case 'R':
            return rook_wh;
            break;
        case 'b':
            return bishop_bl;
            break;
        case 'B':
            return bishop_wh;
            break;
        case 'n':
            return knight_bl;
            break;
        case 'N':
            return knight_wh;
            break;
        case 'q':
            return queen_bl;
            break;
        case 'Q':
            return queen_wh;
            break;
        case 'k':
            return king_bl;
            break;
        case 'K':
            return king_wh;
            break;
    }
}


//switch structures for finding image offset values, to center pieces in squares
int findXImageOffset (char c) {
    switch (c) {
        case 'p': case 'P':
            return XPAWNOFFSET;
            break;
        case 'r': case 'R':
            return XROOKOFFSET;
            break;
        case 'b': case 'B':
            return XBISHOPOFFSET;
            break;
        case 'n': case 'N':
            return XKNIGHTOFFSET;
            break;
        case 'q': case 'Q':
            return XQUEENOFFSET;
            break;
        case 'k': case 'K':
            return XKINGOFFSET;
            break;
    }
}
int findYImageOffset (char c) {
    switch (c) {
        case 'p': case 'P':
            return YPAWNOFFSET;
            break;
        case 'r': case 'R':
            return YROOKOFFSET;
            break;
        case 'b': case 'B':
            return YBISHOPOFFSET;
            break;
        case 'n': case 'N':
            return YKNIGHTOFFSET;
            break;
        case 'q': case 'Q':
            return YQUEENOFFSET;
            break;
        case 'k': case 'K':
            return YKINGOFFSET;
            break;
    }
}

//creates exit button, for use in game screen
void createExitButton()
{
    LCD.SetFontScale(0.5);
    LCD.SetFontColor(GRAY);
    LCD.DrawRectangle(XMAX/2+80, YMAX-40, MenuWidth-125,MenuHeight-10);
    LCD.WriteAt("Exit Game", XMAX/2+90, YMAX-35);
}

//checks if game exit button has been press and sends game back to main menu if yes
void ifExitGame(float x, float y)
{
    //called directly after locate touch, x and y contain click location
    if (coordsInBox(x,y,XMAX/2+80, YMAX-40, MenuWidth-125,MenuHeight-10))
    {
        start(); //return to start menu
    }

}
//ends game when a timer is over - only called when timer is over
void TimerOverProtocol(timer*time) {
    LCD.SetFontColor(WHITE);
    //create game over rectangle
    LCD.FillRectangle(XMAX/2 - END_SCREEN_LENGTH/2-TOP_LEFT_BOARD_X, YMAX/2 - END_SCREEN_LENGTH/2, END_SCREEN_LENGTH,END_SCREEN_LENGTH);
    LCD.SetFontColor(BLACK);
    //display who won
    if(time -> color == 'w') {     
        LCD.WriteAt("Black Won", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    else {
        LCD.WriteAt("White Won", XMAX/2 - TIMER_BOX_X-TOP_LEFT_BOARD_X, YMAX/2 - TIMER_BOX_Y);
    }
    //create exit button and wait for it to be clicked
    createExitButton();
    float x,y;
    locateTouch(&x,&y);
    ifExitGame(x,y);
 }

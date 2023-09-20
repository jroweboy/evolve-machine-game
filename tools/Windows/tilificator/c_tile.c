/*
 * Copyright (C) 2012-2018 Michel Iwaniec
 *
 * This file is part of Tilificator.
 *
 * Tilificator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tilificator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tilificator.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define HASH_TABLE_SIZE 65536

#define TILE_MASK_FLIP_H    0x10000
#define TILE_MASK_FLIP_V    0x20000

#include <Python.h>

PyObject* pyString_x = NULL;
PyObject* pyString_y = NULL;
PyObject* pyString_tileID = NULL;
PyObject* pyString_flipH = NULL;
PyObject* pyString_flipV = NULL;

unsigned char* pTileBuf = NULL;
unsigned int* pHashTable = NULL;

unsigned char rand8[256] = {
  50, 127, 111, 254, 183,  82,  12,  69,  58, 140, 194,  84,  28,  11,  74, 236,
  26,  19, 119,  37, 239, 238,  18, 107,  44, 253, 146, 163,  76, 162, 166, 187,
   7, 136, 217,  29, 113, 128,  97, 116,  83, 120,  86,  62, 184, 159, 115, 201,
  54, 241,  55, 195,  87, 212, 103, 249,  41, 152, 174,  89, 155, 202, 176, 102,
 139, 203,  91, 124,  10, 101, 177, 130, 157, 231,  57, 175, 224, 244, 142, 196,
 209,  77, 189, 141,   1, 255,  52,  81, 225, 114,  24,  67, 137, 109, 227, 199, 
   0,  68,  36,  15, 186,  31, 208, 223, 228, 138,  88,  65, 240,  75, 168, 173,
  94,   8,  40, 246, 125,  61, 247, 121, 206,  22, 112,  93,  99, 226, 172,  20,
 156, 230, 104,  45, 229, 118, 250, 222,   9, 190,  70,  49, 170,  96,   2, 242,
 129, 210,  80, 108,  13,   5, 161,  63,  90, 192,  33, 233, 110, 167, 100, 131,
 106,  34, 213,  43, 219,  47, 153, 232, 143, 251, 198,  25, 193,  38,  95,  42, 
  35,   6, 165, 179, 123, 218,  23, 235, 149, 252, 164,  30, 220, 245, 180, 145,
  27, 188,  32,  73, 160,  39, 158, 134,  79,   4, 178, 171, 248,  56,  59, 148,
  64, 204,  78,  21,   3, 169, 216, 237, 135, 205, 151,  48,  53,  60,  98, 154,
 207, 126,  51, 221, 132, 215, 211, 105,  17, 150, 182,  14, 181,  85,  16, 147,
  92,  46, 117, 191, 197, 133, 144, 200,  72,  71, 234, 122, 214, 243,  66, 185 };

//
// Double hash function, taken from http://oopweb.com/Algorithms/Documents/Sman/Volume/HashTables.html
//
unsigned short int hash(char *str, size_t size) {
    size_t i;
    unsigned short int h;
    unsigned char h1, h2;

    if (size == 0) return 0;
    h1 = *str; 
    h2 = *str + 1;
    
    for(i=1; i<size; i++) {
        h1 = rand8[h1 ^ str[i]];
        h2 = rand8[h2 ^ str[i]];
    }

    // h is in range 0..65535
    h = ((unsigned short int)h1 << 8)|(unsigned short int)h2;

    // use division method to scale
    //return h % HASH_TABLE_SIZE;
    return h & (HASH_TABLE_SIZE-1);
}

static PyObject* hashTile(PyObject *self, PyObject *args, PyObject *kwargs) 
{  
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {"pTile","tileSize",NULL};  
    int tileSize;
    int hashValue;
    
    unsigned char* pTile = NULL;
    Py_buffer tileBuffer;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*i", kwlist, &tileBuffer, &tileSize))
        return NULL;
    pTile = tileBuffer.buf;
        
    if(!pTile || tileSize < 1)
    {
        fprintf(stderr,"c_tile.hashTile: Error in arguments!\n");
        return NULL;
    }
        
    hashValue = hash(pTile, (size_t)tileSize);
   
    PyBuffer_Release(&tileBuffer);
    return Py_BuildValue("i", hashValue);
}

static PyObject* blankTile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    //-----------------------  
    //List arguments/keywords  
    //-----------------------
    static char *kwlist[] = {   "pSpriteImage", "spriteImageWidth", "spriteImageHeight", "x", "y", "tileWidth", "tileHeight", NULL    };

    size_t i,j;
    
    unsigned char* pSpriteImage = NULL;
    Py_buffer spriteImageBuffer;
    int spriteImageWidth = -1;
    int spriteImageHeight = -1;
    int x = -1;
    int y = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*iiiiii:blankTile", kwlist, &spriteImageBuffer, &spriteImageWidth, &spriteImageHeight, &x, &y, &tileWidth, &tileHeight))
        return NULL;

    pSpriteImage = spriteImageBuffer.buf;

    if(!pSpriteImage || spriteImageWidth < 1 || spriteImageHeight < 1 || tileWidth < 1 || tileHeight < 1)
    {
        fprintf(stderr,"c_tile.blankTile: Error in arguments!\n");
        return NULL;
    }

    for(i=0; i<tileHeight; i++) {
        for(j=0; j<tileWidth; j++) {
            int sx = x+j;
            int sy = y+i;
            if(sx >= 0 && sx < spriteImageWidth && sy >= 0 && sy < spriteImageHeight && pSpriteImage[spriteImageWidth*sy+sx])
            {
                PyBuffer_Release(&spriteImageBuffer);
                Py_RETURN_FALSE;
            }
        }
    }
    
    PyBuffer_Release(&spriteImageBuffer);
    Py_RETURN_TRUE;
}



static PyObject* cutTile(PyObject *self, PyObject *args, PyObject *kwargs)
{
    //-----------------------  
    //List arguments/keywords  
    //-----------------------
    static char *kwlist[] = {   "pSpriteImage", "spriteImageWidth", "spriteImageHeight", "x", "y", "pTileData", "tileWidth", "tileHeight", "flipH", "flipV", NULL    };

    size_t i,j;
    
    unsigned char* pSpriteImage = NULL;
    Py_buffer spriteImageBuffer;
    unsigned char* pTileData = NULL;
    Py_buffer tileDataBuffer;
    int spriteImageWidth = -1;
    int spriteImageHeight = -1;
    int x = -1;
    int y = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    int flipH = -1;
    int flipV = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "y*iiiiy*iiii", kwlist, &spriteImageBuffer, &spriteImageWidth, &spriteImageHeight, &x, &y, &tileDataBuffer, &tileWidth, &tileHeight, &flipH, &flipV))
        return NULL;
    pSpriteImage = spriteImageBuffer.buf;
    pTileData = tileDataBuffer.buf;
        
    if(!pSpriteImage || !pTileData || spriteImageWidth < 1 || spriteImageHeight < 1 || tileWidth < 1 || tileHeight < 1 || flipH < 0 || flipV < 0)
    {
        fprintf(stderr,"c_tile.cutTile: Error in arguments!\n");
        return NULL;
    }

    if(!flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                int sx = x+j;
                int sy = y+i;
                if(sx >= 0 && sx < spriteImageWidth && sy >= 0 && sy < spriteImageHeight && pSpriteImage[spriteImageWidth*sy+sx])
                    pTileData[tileWidth*i+j] = pSpriteImage[spriteImageWidth*sy+sx];
                else
                    pTileData[tileWidth*i+j] = 0;
            }
        }
        PyBuffer_Release(&spriteImageBuffer);
        PyBuffer_Release(&tileDataBuffer);
        Py_RETURN_NONE;
    }
    else if(flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                int sx = x+j;
                int sy = y+i;
                if(sx >= 0 && sx < spriteImageWidth && sy >= 0 && sy < spriteImageHeight && pSpriteImage[spriteImageWidth*sy+sx])
                    pTileData[tileWidth*i+tileWidth-1-j] = pSpriteImage[spriteImageWidth*sy+sx];
                else
                    pTileData[tileWidth*i+tileWidth-1-j] = 0;
            }
        }
        PyBuffer_Release(&spriteImageBuffer);
        PyBuffer_Release(&tileDataBuffer);
        Py_RETURN_NONE;
    }
    else if(!flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                int sx = x+j;
                int sy = y+i;
                if(sx >= 0 && sx < spriteImageWidth && sy >= 0 && sy < spriteImageHeight && pSpriteImage[spriteImageWidth*sy+sx])
                    pTileData[tileWidth*(tileHeight-1-i)+j] = pSpriteImage[spriteImageWidth*sy+sx];
                else
                    pTileData[tileWidth*(tileHeight-1-i)+j] = 0;
            }
        }
        PyBuffer_Release(&spriteImageBuffer);
        PyBuffer_Release(&tileDataBuffer);
        Py_RETURN_NONE;
    }
    else { // if !flipH && !flipV
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                int sx = x+j;
                int sy = y+i;
                if(sx >= 0 && sx < spriteImageWidth && sy >= 0 && sy < spriteImageHeight && pSpriteImage[spriteImageWidth*sy+sx])
                    pTileData[tileWidth*(tileHeight-1-i)+tileWidth-1-j] = pSpriteImage[spriteImageWidth*sy+sx];
                else
                    pTileData[tileWidth*(tileHeight-1-i)+tileWidth-1-j] = 0;
            }
        }
        PyBuffer_Release(&spriteImageBuffer);
        PyBuffer_Release(&tileDataBuffer);
        Py_RETURN_NONE;
    }
    PyBuffer_Release(&spriteImageBuffer);
    PyBuffer_Release(&tileDataBuffer);
    Py_RETURN_NONE;
}

int compareTilesSimple(unsigned char* tileA, unsigned char* tileB, size_t tileSize)
{
    size_t i;
    for(i=0; i<tileSize; i++)
        if(tileA[i] != tileB[i])
            return 0;
    return 1;
}

int compareTiles(unsigned char* tileA, unsigned char* tileB, int tileWidth, int tileHeight, int flipH, int flipV)
{
    size_t i,j;

    if(!flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(tileA[i*tileWidth+j] != tileB[i*tileWidth+j])
                    return 0;
            }
        }
        return 1;
    }
    else if(flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(tileA[i*tileWidth+j] != tileB[i*tileWidth+tileWidth-1-j])
                    return 0;
            }
        }
        return 1;
    }
    else if(!flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(tileA[i*tileWidth+j] != tileB[(tileHeight-1-i)*tileWidth+j])
                    return 0;
            }
        }
        return 1;
    }
    else { //if(flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(tileA[i*tileWidth+j] != tileB[(tileHeight-1-i)*tileWidth+tileWidth-1-j])
                    return 0;
            }
        }
        return 1;
    }
}

int compareTileToImage(unsigned char* pTile, int tileWidth, int tileHeight, unsigned char* pImage, int imageWidth, int imageHeight, int flipH, int flipV)
{
    size_t i,j,offs=0;

    if(!flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(pTile[offs++] != pImage[i*imageWidth+j])
                    return 0;
            }
        }
        return 1;
    }
    else if(flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(pTile[offs++] != pImage[i*imageWidth+tileWidth-1-j])
                    return 0;
            }
        }
        return 1;
    }
    else if(!flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(pTile[offs++] != pImage[(tileHeight-1-i)*imageWidth+j])
                    return 0;
            }
        }
        return 1;
    }
    else { //if(flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                if(pTile[offs++] != pImage[(tileHeight-1-i)*imageWidth+tileWidth-1-j])
                    return 0;
            }
        }
        return 1;
    }
}

void add_tile(unsigned char* pTileTable, unsigned char* pTile, int numTiles, int tileWidth, int tileHeight)
{
    static int maxIterations = 1;
    int numIterations = 0;
    size_t i, j;
    int tileIndex = 0;
    int h;    
    int tileSize = tileWidth*tileHeight;
    unsigned char* pTileFlipNone = pTileBuf;
    unsigned char* pTileFlipH = pTileBuf+tileSize;
    unsigned char* pTileFlipV = pTileBuf+tileSize*2;
    unsigned char* pTileFlipHV = pTileBuf+tileSize*3;
    for(i=0; i<tileHeight; i++)
    {
        for(j=0; j<tileWidth; j++)
        {
            pTileFlipNone[i*tileWidth+j] = pTile[i*tileWidth+j];
            pTileFlipH[i*tileWidth+(tileWidth-1-j)] = pTile[i*tileWidth+j];            
            pTileFlipV[(tileHeight-1-i)*tileWidth+j] = pTile[i*tileWidth+j];
            pTileFlipHV[(tileHeight-1-i)*tileWidth+(tileWidth-1-j)] = pTile[i*tileWidth+j];
        }
    }    
    
    //fprintf(stderr,"add_tile: tileSize = %d\n",(int)tileSize);
    //fprintf(stderr,"add_tile: hash = %d\n",(int)hash(pTile, tileSize));
    for(h = hash(pTile, tileSize); pHashTable[h] != -1; h=(h+1) & (HASH_TABLE_SIZE-1)) {
        //fprintf(stderr,"          h = %d\n",(int)h);
        //tileIndex = pHashTable[h];
        numIterations++;
    }
    pHashTable[h] = numTiles;
    for(h = hash(pTileFlipH, tileSize); pHashTable[h] != -1; h=(h+1) & (HASH_TABLE_SIZE-1)) {
        //fprintf(stderr,"          h = %d\n",(int)h);
        //tileIndex = pHashTable[h];
        numIterations++;
    }
    pHashTable[h] = numTiles | TILE_MASK_FLIP_H;
    for(h = hash(pTileFlipV, tileSize); pHashTable[h] != -1; h=(h+1) & (HASH_TABLE_SIZE-1)) {
        //fprintf(stderr,"          h = %d\n",(int)h);
        //tileIndex = pHashTable[h];
        numIterations++;
    }
    pHashTable[h] = numTiles | TILE_MASK_FLIP_V;
    for(h = hash(pTileFlipHV, tileSize); pHashTable[h] != -1; h=(h+1) & (HASH_TABLE_SIZE-1)) {
        //fprintf(stderr,"          h = %d\n",(int)h);
        //tileIndex = pHashTable[h];
        numIterations++;
    }
    pHashTable[h] = numTiles | TILE_MASK_FLIP_H | TILE_MASK_FLIP_V;
    
    if(numIterations > maxIterations) {
        maxIterations = numIterations;
        //fprintf(stderr,"add_tile: maxIterations = %d\n",(int)maxIterations);
    }
    
    for(i=0; i<tileSize; i++) {
        pTileTable[numTiles*tileSize+i] = pTile[i];
    }
}

void remove_tiles(unsigned char* pTileTable, int numTiles, int tileWidth, int tileHeight, int numTilesToRemove)
{
    static int maxIterations = 1;
    int numIterations = 0;
    int h;
    size_t i,j;
    int tileSize = tileWidth*tileHeight;
    unsigned char* pTileFlipNone = pTileBuf;
    unsigned char* pTileFlipH = pTileBuf+tileSize;
    unsigned char* pTileFlipV = pTileBuf+tileSize*2;
    unsigned char* pTileFlipHV = pTileBuf+tileSize*3;

    //fprintf(stderr,"remove_tiles: tileSize = %d\n",(int)tileSize);
    //fprintf(stderr,"remove_tiles: hash = %d\n",(int)hash(&pTileTable[(numTiles-1)*tileSize], tileSize));
    while(numTilesToRemove) {
        unsigned char* pTile = &pTileTable[(numTiles-1)*tileSize];
        for(i=0; i<tileHeight; i++)
        {
            for(j=0; j<tileWidth; j++)
            {
                pTileFlipNone[i*tileWidth+j] = pTile[i*tileWidth+j];
                pTileFlipH[i*tileWidth+(tileWidth-1-j)] = pTile[i*tileWidth+j];            
                pTileFlipV[(tileHeight-1-i)*tileWidth+j] = pTile[i*tileWidth+j];
                pTileFlipHV[(tileHeight-1-i)*tileWidth+(tileWidth-1-j)] = pTile[i*tileWidth+j];
            }
        }    

        numIterations=0;
        for(h = hash(pTileFlipNone, tileSize); ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
            //fprintf(stderr,"remove_tiles: h = %d\n",(int)h);
            if(pHashTable[h] == numTiles-1) {
                pHashTable[h] = -1;
                break;
            }
            numIterations++;
        }
        for(h = hash(pTileFlipH, tileSize); ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
            //fprintf(stderr,"remove_tiles: h = %d\n",(int)h);
            if(pHashTable[h] == ((numTiles-1) | TILE_MASK_FLIP_H)) {
                pHashTable[h] = -1;
                break;
            }
            numIterations++;
        }
        for(h = hash(pTileFlipV, tileSize); ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
            //fprintf(stderr,"remove_tiles: h = %d\n",(int)h);
            if(pHashTable[h] == ((numTiles-1) | TILE_MASK_FLIP_V)) {
                pHashTable[h] = -1;
                break;
            }
            numIterations++;
        }

        for(h = hash(pTileFlipHV, tileSize); ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
            //fprintf(stderr,"remove_tiles: h = %d\n",(int)h);
            if(pHashTable[h] == ((numTiles-1) | TILE_MASK_FLIP_H | TILE_MASK_FLIP_V)) {
                pHashTable[h] = -1;
                break;
            }
            numIterations++;
        }

        if(numIterations > maxIterations) {
            maxIterations = numIterations;
            //fprintf(stderr,"remove_tiles: maxIterations = %d\n",(int)maxIterations);
        }
        numTilesToRemove--;
        numTiles--;
    }
}

int find_tile(unsigned char* pTileTable, unsigned char* pTile, int numTiles, size_t tileWidth, size_t tileHeight)
{
    static int maxIterations = 1;
    int numIterations = 0;
    int h;
    size_t tileSize = tileWidth*tileHeight;
    for(h = hash(pTile, tileSize); ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
        int tileIndex = pHashTable[h];
        if(tileIndex == -1 || compareTiles(pTile, &pTileTable[(tileIndex & 0xFFFF)*tileSize], tileWidth, tileHeight, tileIndex & TILE_MASK_FLIP_H, tileIndex & TILE_MASK_FLIP_V)) {
            if(numIterations > maxIterations) {
                maxIterations = numIterations;
                //fprintf(stderr,"find_tile: maxIterations = %d\n",(int)maxIterations);
            }
            return tileIndex;
        }
        numIterations++;
    }

    if(numIterations > maxIterations) {
        maxIterations = numIterations;
        //fprintf(stderr,"find_tile: maxIterations = %d\n",(int)maxIterations);
    }

    return -1;
}

static PyObject* addTile(PyObject *self, PyObject *args, PyObject *kwargs) 
{  
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {"pTile","pTileTable","numTiles","tileWidth","tileHeight",NULL};  
    size_t i,j;
    size_t tileSize;
    
    unsigned char* pTile = NULL;
    Py_buffer tileBuffer;
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    int numTiles = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*iii", kwlist, &tileBuffer, &tileTableBuffer, &numTiles, &tileWidth, &tileHeight))
        return NULL;
    pTile = tileBuffer.buf;
    pTileTable = tileTableBuffer.buf;
        
    if(!pTile || !pTileTable || numTiles < 0 || tileWidth < 1 || tileHeight < 1)
    {
        fprintf(stderr,"c_tile.addTile: Error in arguments!\n");
        return NULL;
    }
    
    //fprintf(stderr,"addTile: tileWidth = %d, tileHeight = %d, tileSize = %d\n",tileWidth,tileHeight,(int)tileSize);
    
    add_tile(pTileTable, pTile, numTiles, tileWidth, tileHeight);
    
    PyBuffer_Release(&tileBuffer);
    PyBuffer_Release(&tileTableBuffer);
    Py_RETURN_NONE;
}



static PyObject* removeTiles(PyObject *self, PyObject *args, PyObject *kwargs) 
{  
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {"pTileTable","tileWidth","tileHeight","numTiles","numTilesToRemove",NULL};
    size_t i,j;
    size_t tileSize;
    
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    int numTiles = -1;
    int numTilesToRemove = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*iiii", kwlist, &tileTableBuffer, &tileWidth, &tileHeight, &numTiles, &numTilesToRemove))
        return NULL;
    pTileTable = tileTableBuffer.buf;
        
    if(!pTileTable || tileWidth < 1 || tileHeight < 1 || numTiles < 0 || numTilesToRemove < 1)
    {
        fprintf(stderr,"c_tile.removeTiles: Error in arguments!\n");
        return NULL;
    }
    
    remove_tiles(pTileTable, numTiles, tileWidth, tileHeight, numTilesToRemove);

    PyBuffer_Release(&tileTableBuffer);
    Py_RETURN_NONE;
}



static PyObject* findTile(PyObject *self, PyObject *args, PyObject *kwargs) 
{  
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {"pTile","pTileTable","numTiles","tileWidth","tileHeight","enableFlipH","enableFlipV",NULL};  
    size_t i,j;
    int equal;
    int equalTileID = -1;
    size_t tileSize;
    
    unsigned char* pTile = NULL;
    Py_buffer tileBuffer;
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    int numTiles = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    int enableFlipH = -1;
    int enableFlipV = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*iiiii", kwlist, &tileBuffer, &tileTableBuffer, &numTiles, &tileWidth, &tileHeight, &enableFlipH, &enableFlipV))
        return NULL;
    pTile = tileBuffer.buf;
    pTileTable = tileTableBuffer.buf;
        
    if(!pTile || !pTileTable || numTiles < 0 || tileWidth < 1 || tileHeight < 1 || enableFlipH < 0 || enableFlipV < 0)
    {
        fprintf(stderr,"c_tile.findTile: Error in arguments!\n");
        return NULL;
    }    
    tileSize = tileWidth*tileHeight;
    
    // Non-flipped
    equalTileID = find_tile(pTileTable, pTile, numTiles, tileWidth, tileHeight);
    PyBuffer_Release(&tileBuffer);
    PyBuffer_Release(&tileTableBuffer);
    if(equalTileID >= 0)
        return Py_BuildValue("iOO", 
                            equalTileID & 0xFFFF, 
                            (equalTileID & TILE_MASK_FLIP_H)?Py_True:Py_False, 
                            (equalTileID & TILE_MASK_FLIP_V)?Py_True:Py_False);
    //----------------  
    //Return equalTileID
    //----------------  
    return Py_BuildValue("iOO", equalTileID, Py_False, Py_False);
}



typedef struct _MatchedTile
{
    int x,y,tileID,flipH,flipV;
} MatchedTile;

MatchedTile* pMatchedTiles = NULL;
unsigned char* pImageBuf = NULL;
unsigned char* imageCoverage = NULL;
unsigned char* tileBuf = NULL;
int numTiles = 0;

int numMatchedTiles = 0;

void match_tile(int imageWidth, int imageHeight, int tileWidth, int tileHeight, int tileID)
{
    int x,y,xx,yy;
    int tileSize = tileWidth*tileHeight;
    //unsigned char* tileBuf = pTileBuf;
    for(y=0; y<imageHeight-tileHeight; y++) {
        for(x=0; x<imageWidth; x++) {
            int tileOffs = 0; //tileID*tileSize;
            int imageOffs = y*imageWidth+x;
            int matched = 1, matchedFlipH = 1, matchedFlipV = 1, matchedFlipHV = 1;
            for(yy=0; yy<tileHeight; yy++) {
                for(xx=0; xx<tileWidth; xx++) {
                    //if(tileBuf[tileOffs+xx]) {
                        //if(tileBuf[tileOffs+xx] == pImageBuf[imageOffs+xx]) {
                        //    matched = 2;
                        //}
                        //else if(tileBuf[tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) {
                        //    matchedFlipH = 2;
                        //}
                        //else if(tileBuf[tileSize-tileHeight-tileOffs+xx] == pImageBuf[imageOffs+xx]) {
                        //    matchedFlipV = 2;
                        //}
                        //else if(tileBuf[tileSize-tileHeight-tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) {
                        //    matchedFlipHV = 2;
                        //}
                        //else {                        
                        //    matched = 0;
                        //    break;
                        //}
                        matched &= ((tileBuf[tileOffs+xx] == pImageBuf[imageOffs+xx]) & 1) | ((!tileBuf[tileOffs+xx]) &1);
                        matchedFlipH &= ((tileBuf[tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) & 1) | ((!tileBuf[tileOffs+tileWidth-1-xx]) &1);
                        matchedFlipV &= ((tileBuf[tileSize-tileWidth-tileOffs+xx] == pImageBuf[imageOffs+xx]) & 1) | ((!tileBuf[tileSize-tileWidth-tileOffs+xx]) &1);
                        matchedFlipHV &= ((tileBuf[tileSize-tileWidth-tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) & 1) | ((!tileBuf[tileSize-tileWidth-tileOffs+tileWidth-1-xx]) &1);
                    //}
                }
                if(!matched && !matchedFlipH && !matchedFlipV && !matchedFlipHV)
                    break;
                tileOffs+=tileWidth;
                imageOffs+=imageWidth;
            }
            if(matched) //if(matched == 2)
            {
                pMatchedTiles[numMatchedTiles].x = x;
                pMatchedTiles[numMatchedTiles].y = y;
                pMatchedTiles[numMatchedTiles].tileID = tileID;
                pMatchedTiles[numMatchedTiles].flipH = 0;
                pMatchedTiles[numMatchedTiles].flipV = 0;
                numMatchedTiles++;
                // Mark non-transparent pixels as covered
                imageOffs = y*imageWidth+x;
                tileOffs = 0;
                for(yy=0; yy<tileHeight; yy++)
                {
                    for(xx=0; xx<tileWidth; xx++)
                    {
                        if(tileBuf[tileOffs+xx] && ((tileBuf[tileOffs+xx] == pImageBuf[imageOffs+xx]) & 1))
                            imageCoverage[imageWidth*(y+yy)+x+xx]++;
                        //if(pImageBuf[imageWidth*(y+yy)+x+xx])
                    }
                    tileOffs+=tileWidth;
                    imageOffs+=imageWidth;
                }
            }
            else if(matchedFlipH)
            {
                pMatchedTiles[numMatchedTiles].x = x;
                pMatchedTiles[numMatchedTiles].y = y;
                pMatchedTiles[numMatchedTiles].tileID = tileID;
                pMatchedTiles[numMatchedTiles].flipH = 1;
                pMatchedTiles[numMatchedTiles].flipV = 0;
                numMatchedTiles++;
                // Mark non-transparent pixels as covered
                imageOffs = y*imageWidth+x;
                tileOffs = 0;
                for(yy=0; yy<tileHeight; yy++)
                {
                    for(xx=0; xx<tileWidth; xx++)
                    {
                        if(tileBuf[tileOffs+tileWidth-1-xx] && ((tileBuf[tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) & 1))
                            imageCoverage[imageWidth*(y+yy)+x+xx]++;
                        //if(pImageBuf[imageWidth*(y+yy)+x+tileWidth-1-xx])
                        //    imageCoverage[imageWidth*(y+yy)+x+tileWidth-1-xx]++;
                    }
                    tileOffs+=tileWidth;
                    imageOffs+=imageWidth;
                }
            }
            else if(matchedFlipV)
            {
                pMatchedTiles[numMatchedTiles].x = x;
                pMatchedTiles[numMatchedTiles].y = y;
                pMatchedTiles[numMatchedTiles].tileID = tileID;
                pMatchedTiles[numMatchedTiles].flipH = 0;
                pMatchedTiles[numMatchedTiles].flipV = 1;
                numMatchedTiles++;
                // Mark non-transparent pixels as covered
                imageOffs = y*imageWidth+x;
                tileOffs = 0;
                for(yy=0; yy<tileHeight; yy++)
                {
                    for(xx=0; xx<tileWidth; xx++)
                    {
                        if(tileBuf[tileSize-tileWidth-tileOffs+xx] && ((tileBuf[tileSize-tileWidth-tileOffs+xx] == pImageBuf[imageOffs+xx]) & 1))
                            imageCoverage[imageWidth*(y+yy)+x+xx]++;
                        //if(pImageBuf[imageWidth*(y+yy+tileHeight-1-yy)+x+xx])
                        //    imageCoverage[imageWidth*(y+yy+tileHeight-1-yy)+x+xx]++;
                    }
                    tileOffs+=tileWidth;
                    imageOffs+=imageWidth;
                }
            }
            else if(matchedFlipHV)
            {
                pMatchedTiles[numMatchedTiles].x = x;
                pMatchedTiles[numMatchedTiles].y = y;
                pMatchedTiles[numMatchedTiles].tileID = tileID;
                pMatchedTiles[numMatchedTiles].flipH = 1;
                pMatchedTiles[numMatchedTiles].flipV = 1;
                numMatchedTiles++;
                // Mark non-transparent pixels as covered
                imageOffs = y*imageWidth+x;
                tileOffs = 0;
                for(yy=0; yy<tileHeight; yy++)
                {
                    for(xx=0; xx<tileWidth; xx++)
                    {
                        if(tileBuf[tileSize-tileWidth-tileOffs+tileWidth-1-xx] && ((tileBuf[tileSize-tileWidth-tileOffs+tileWidth-1-xx] == pImageBuf[imageOffs+xx]) & 1))
                            imageCoverage[imageWidth*(y+yy)+x+xx]++;
                        //if(pImageBuf[imageWidth*(y+yy+tileHeight-1-yy)+x+tileWidth-1-xx])
                        //    imageCoverage[imageWidth*(y+yy+tileHeight-1-yy)+x+tileWidth-1-xx]++;
                    }
                    tileOffs+=tileWidth;
                    imageOffs+=imageWidth;
                }
            }
        }
    }
}

int blankTileData(int tileWidth, int tileHeight, unsigned char* tileData)
{
    int i,j;
    for(i=0; i<tileHeight; i++)
        for(j=0; j<tileWidth; j++)
            if(tileData[i*tileWidth+j] != 0)
                return 0;
    return 1;
}

static PyObject* matchTiles(PyObject *self, PyObject *args, PyObject *kwargs)
{  
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {   "pSpriteImage","pSpriteImageCoverage","spriteImageWidth","spriteImageHeight",
                                "pTileTable","numTiles","tileWidth","tileHeight","enableFlipH","enableFlipV",NULL};  
    size_t i,j,n;
    int equal;
    size_t tileSize;
    
    unsigned char* pTileFlipNone = NULL;
    int numTiles = -1;
    int spriteImageWidth = -1;
    int spriteImageHeight = -1;
    int enableFlipH = -1;
    int enableFlipV = -1;
    unsigned char* pSpriteImage = NULL;
    Py_buffer spriteImageBuffer;
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    unsigned char* pSpriteImageCoverage = NULL;
    Py_buffer spriteImageCoverageBuffer;
    int tileWidth = -1;
    int tileHeight = -1;
    int imageWidth = 0;
    int imageHeight = 0;
    PyObject* pTilifications = NULL;

    //---------------  
    //Parse the input  
    //---------------  
    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "y*y*iiy*iiiii", kwlist, 
                                    &spriteImageBuffer, &spriteImageCoverageBuffer, &spriteImageWidth, &spriteImageHeight, 
                                    &tileTableBuffer, &numTiles, &tileWidth, &tileHeight,
                                    &enableFlipH, &enableFlipV))
        return NULL;
    pSpriteImage = spriteImageBuffer.buf;
    pSpriteImageCoverage = spriteImageCoverageBuffer.buf;
    pTileTable = tileTableBuffer.buf;

    if( !pSpriteImage || !pSpriteImageCoverage || spriteImageWidth < 1 || spriteImageHeight < 1 || 
        !pTileTable || numTiles < 0 || tileWidth < 1 || tileHeight < 1 || 
        enableFlipH < 0 || enableFlipV < 0)
    {
        fprintf(stderr,"c_tile.matchTiles: Error in arguments!\n");
        return NULL;
    }

    
    // Create padded image for simplification/optimization(?) (might actually run slower... investigate this further!)
    imageWidth = spriteImageWidth+tileWidth;
    imageHeight = spriteImageHeight+tileHeight*2;

    for(i=0; i<imageWidth*imageHeight; i++)
        pImageBuf[i] = 0;

    for(i=0; i<spriteImageHeight; i++) {
        for(j=0; j<spriteImageWidth; j++) {
            pImageBuf[(i+tileHeight)*imageWidth+j+tileWidth] = pSpriteImage[i*spriteImageWidth+j];
            imageCoverage[(i+tileHeight)*imageWidth+j+tileWidth] = 0;
        }
    }

    tileSize = tileWidth*tileHeight;
    numMatchedTiles = 0;
    for(n=0; n<numTiles; n++)
    {
        pTileFlipNone = &pTileTable[tileSize*n];
        tileBuf = pTileFlipNone;
        //printf("Calling match_tile for n=%d...\n",n);
        if(!blankTileData(tileWidth, tileHeight, tileBuf))
            match_tile(imageWidth, imageHeight, tileWidth, tileHeight, n);
        //printf("  done!\n");
    }

    //printf("Building list...\n");

    pTilifications = PyList_New(numMatchedTiles);
    
    for(i=0; i<numMatchedTiles; i++) {
        MatchedTile t = pMatchedTiles[i];
        PyList_SetItem(pTilifications, i, Py_BuildValue("iiiOO", t.x-tileWidth, t.y-tileHeight, t.tileID, t.flipH?Py_True:Py_False, t.flipV?Py_True:Py_False));
    }
    
    //printf("Built list\n");

    for(i=0; i<spriteImageHeight; i++) {
        for(j=0; j<spriteImageWidth; j++) {
            pSpriteImageCoverage[i*spriteImageWidth+j] = imageCoverage[(i+tileHeight)*imageWidth+j+tileWidth]; //pImageBuf[(i+tileHeight)*imageWidth+j+tileWidth];
        }
    }
    PyBuffer_Release(&spriteImageBuffer);
    PyBuffer_Release(&tileTableBuffer);
    PyBuffer_Release(&spriteImageCoverageBuffer);
    return Py_BuildValue("O", pTilifications); //, imageCoverage);
}

static PyObject* getAllMatches(PyObject *self, PyObject *args, PyObject *kwargs)
{
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {   "xStart","yStart","xEnd","yEnd",
                                "pSpriteImage","pHashImage","spriteImageWidth","spriteImageHeight",
                                "pTileTable","numTiles","tileWidth","tileHeight","enableFlipH","enableFlipV",NULL};  
    unsigned char* pTileFlipNone = NULL;
    unsigned char* pTileFlipH = NULL;
    unsigned char* pTileFlipV = NULL;
    unsigned char* pTileFlipHV = NULL;
    unsigned char c;
    int xStart, yStart, xEnd, yEnd;
    unsigned char* pSpriteImage = NULL;
    Py_buffer spriteImageBuffer;
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    unsigned short* pHashImage = NULL;
    Py_buffer hashImageBuffer;
    int spriteImageWidth, spriteImageHeight, hashImageWidth, hashImageHeight;
    int numTiles, tileWidth, tileHeight;
    int enableFlipH, enableFlipV;
    PyObject* matchesListList = NULL;
    PyObject* matchesList = NULL;
    int x, y, i, j, tileSize, tileID, h;

    //---------------  
    //Parse the input  
    //---------------  
    if(!PyArg_ParseTupleAndKeywords(args, kwargs, "iiiiy*y*iiy*iiiii", kwlist,
                                    &xStart, &yStart, &xEnd, &yEnd,
                                    &spriteImageBuffer, &hashImageBuffer, &spriteImageWidth, &spriteImageHeight, 
                                    &tileTableBuffer, &numTiles, &tileWidth, &tileHeight,
                                    &enableFlipH, &enableFlipV))
        return NULL;
    pSpriteImage = spriteImageBuffer.buf;
    pHashImage = hashImageBuffer.buf;
    pTileTable = tileTableBuffer.buf;
        
    if( !pSpriteImage || !pHashImage || spriteImageWidth < 1 || spriteImageHeight < 1 || 
        !pTileTable || numTiles < 0 || tileWidth < 1 || tileHeight < 1 || 
        enableFlipH < 0 || enableFlipV < 0)
    {
        fprintf(stderr,"c_tile.matchTiles: Error in arguments!\n");
        return NULL;
    }
    tileSize = tileWidth*tileHeight;
    hashImageWidth = spriteImageWidth;
    hashImageHeight = spriteImageHeight;

    matchesListList = PyList_New(0);
    for(y=yStart; y<=yEnd; y++) {
        matchesList = PyList_New(0);
        for(x=xStart; x<=xEnd; x++) {
            h = pHashImage[y*hashImageWidth+x];
            tileID = -1;
            for(; ; h=(h+1) & (HASH_TABLE_SIZE-1)) {
                tileID = pHashTable[h];
                if(tileID == -1 || compareTileToImage(&pTileTable[(tileID & 0xFFFF)*tileSize], tileWidth, tileHeight, &pSpriteImage[y*spriteImageWidth+x], spriteImageWidth, spriteImageHeight, tileID & TILE_MASK_FLIP_H, tileID & TILE_MASK_FLIP_V)) 
                    break;
            }
            if(tileID != -1) {
                PyObject* match = Py_BuildValue("iiiOO", x, y, tileID & 0xFFFF, (tileID & TILE_MASK_FLIP_H)?Py_True:Py_False, (tileID & TILE_MASK_FLIP_V)?Py_True:Py_False);
                PyList_Append(matchesList, match);
                Py_DECREF(match);
                continue;
            }
        }
        if(PyList_Size(matchesList) > 0) {
            PyObject* tuple = Py_BuildValue("iN", y, matchesList);
            PyList_Append(matchesListList, tuple);
            Py_DECREF(tuple);
        }
        else
            Py_CLEAR(matchesList);
    }
    PyBuffer_Release(&spriteImageBuffer);
    PyBuffer_Release(&hashImageBuffer);
    PyBuffer_Release(&tileTableBuffer);
    return matchesListList;
}

/* 1785 => 1741 */
static PyObject* coverageFromTilification(PyObject *self, PyObject *args, PyObject *kwargs)
{
    //-----------------------  
    //List arguments/keywords  
    //-----------------------  
    static char *kwlist[] = {"coverage","spriteTiles","pImage","imageWidth","imageHeight","pTileTable","tileWidth","tileHeight","binary",NULL};  
    size_t tileSize;    
    //unsigned char* pTile = NULL;
    unsigned char* pTileTable = NULL;
    Py_buffer tileTableBuffer;
    unsigned char* pImage = NULL;
    Py_buffer imageBuffer;
    unsigned char* pCoverage = NULL;
    Py_buffer coverageBuffer;
    int numTiles = -1;
    int tileWidth = -1;
    int tileHeight = -1;
    int imageWidth = -1;
    int imageHeight = -1;
    int width = -1;
    int height = -1;
    int binary = -1;
    PyObject* spriteTilesList = NULL;
    PyObject* st = NULL;
    int i, j, k;
    unsigned char c;
    int numSpriteTiles = -1;
    
    //---------------  
    //Parse the input  
    //---------------  
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "y*Oy*iiy*iii", kwlist, &coverageBuffer, &spriteTilesList, &imageBuffer, &imageWidth, &imageHeight, &tileTableBuffer, &tileWidth, &tileHeight, &binary))
        return NULL;
    pTileTable = tileTableBuffer.buf;
    pImage = imageBuffer.buf;
    pCoverage = coverageBuffer.buf;
    
    if(!pTileTable || !pImage || !pCoverage || imageWidth < 1 || imageHeight < 1 || tileWidth < 1 || tileHeight < 1)
    {
        fprintf(stderr,"c_tile.coverageFromTilification: Error in arguments!\n");
        return NULL;
    }

    width = imageWidth;
    height = imageHeight;
    c = binary?0:0xFF;
    numSpriteTiles = PySequence_Size(spriteTilesList);

    for(i = 0; i < imageWidth*imageHeight; i++)
        pCoverage[i] = 0;
    
    for(k = 0; k < numSpriteTiles; k++)
    {
        PyObject* st = PySequence_GetItem(spriteTilesList, k);
        int tx = PyLong_AS_LONG(PyObject_GetAttr(st, pyString_x)); /*PyInt_AS_LONG(PyObject_GetAttrString(st, "x"));*/
        int ty = PyLong_AS_LONG(PyObject_GetAttr(st, pyString_y)); /*PyInt_AS_LONG(PyObject_GetAttrString(st, "y"));*/
        int tID = PyLong_AS_LONG(PyObject_GetAttr(st, pyString_tileID)); /*PyInt_AS_LONG(PyObject_GetAttrString(st, "tileID"));*/
        int flipH = PyLong_AS_LONG(PyObject_GetAttr(st, pyString_flipH)); /*PyObject_IsTrue(PyObject_GetAttrString(st, "flipH"));*/
        int flipV = PyLong_AS_LONG(PyObject_GetAttr(st, pyString_flipV)); /*PyObject_IsTrue(PyObject_GetAttrString(st, "flipV"));*/
        unsigned char* data = &pTileTable[tID*tileWidth*tileHeight];
                    
    if(!flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                unsigned char cTile = data[tileWidth*i+j];
                if(tx+j >= 0 && tx+j < imageWidth && ty+i >= 0 && ty+i < imageHeight && cTile != 0)
                    pCoverage[(ty+i)*imageWidth + tx+j] = (c & pCoverage[(ty+i)*width + tx+j])+1;
            }
        }
    }
    else if(flipH && !flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                unsigned char cTile = data[tileWidth*i+tileWidth-1-j];
                if(tx+j >= 0 && tx+j < imageWidth && ty+i >= 0 && ty+i < imageHeight && cTile != 0)
                    pCoverage[(ty+i)*imageWidth + tx+j] = (c & pCoverage[(ty+i)*width + tx+j])+1;
            }
        }
    }
    else if(!flipH && flipV) {
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                unsigned char cTile = data[tileWidth*(tileHeight-1-i)+j];
                if(tx+j >= 0 && tx+j < imageWidth && ty+i >= 0 && ty+i < imageHeight && cTile != 0)
                    pCoverage[(ty+i)*imageWidth + tx+j] = (c & pCoverage[(ty+i)*width + tx+j])+1;
            }
        }
    }
    else { // if !flipH && !flipV
        for(i=0; i<tileHeight; i++) {
            for(j=0; j<tileWidth; j++) {
                unsigned char cTile = data[tileWidth*(tileHeight-1-i)+tileWidth-1-j];
                if(tx+j >= 0 && tx+j < imageWidth && ty+i >= 0 && ty+i < imageHeight && cTile != 0)
                    pCoverage[(ty+i)*imageWidth + tx+j] = (c & pCoverage[(ty+i)*width + tx+j])+1;
            }
        }
    }
    }
    PyBuffer_Release(&tileTableBuffer);
    PyBuffer_Release(&imageBuffer);
    PyBuffer_Release(&coverageBuffer);
    Py_RETURN_NONE;
}

 struct module_state {
    PyObject *error;
};

static PyMethodDef c_tileMethods[] = {
    {"hashTile", (PyCFunction)hashTile, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"blankTile", (PyCFunction)blankTile, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"cutTile", (PyCFunction)cutTile, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"findTile", (PyCFunction)findTile, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"addTile", (PyCFunction)addTile, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"removeTiles", (PyCFunction)removeTiles, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"matchTiles", (PyCFunction)matchTiles, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"getAllMatches", (PyCFunction)getAllMatches, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {"coverageFromTilification", (PyCFunction)coverageFromTilification, METH_VARARGS|METH_KEYWORDS, "HELP for c_tile\n"},
    {NULL,NULL,0,NULL} /* Sentinel -- don't change*/
};

static int c_tileTraverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(((struct module_state*)PyModule_GetState(m))->error);
    return 0;
}

static int c_tileClear(PyObject *m) {
    Py_CLEAR(((struct module_state*)PyModule_GetState(m))->error);
    return 0;
}

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "c_tile",
        NULL,
        sizeof(struct module_state),
        c_tileMethods,
        NULL,
        c_tileTraverse,
        c_tileClear,
        NULL
};

PyMODINIT_FUNC
PyInit_c_tile(void)
{
    size_t i;

    PyObject *module = PyModule_Create(&moduledef);

    pTileBuf = malloc(256*256*4);
    pHashTable = malloc(HASH_TABLE_SIZE*sizeof(int));
    for(i=0; i<HASH_TABLE_SIZE; i++)
        pHashTable[i] = -1;
    pMatchedTiles = malloc(sizeof(MatchedTile)*65536);
    pImageBuf = malloc(1024*1024);
    imageCoverage = malloc(1024*1024);
    pyString_x = PyUnicode_InternFromString("x");
    pyString_y = PyUnicode_InternFromString("y");
    pyString_tileID = PyUnicode_InternFromString("tileID");
    pyString_flipH = PyUnicode_InternFromString("flipH");
    pyString_flipV = PyUnicode_InternFromString("flipV");
    //import_array();

    return module;
}


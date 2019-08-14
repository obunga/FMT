/*
MIT License

Copyright (c) [2019] [Bureau du forestier en chef]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "FMTcoordinate.h"
#include <iostream>

namespace Spatial
    {
	vector<int>FMTcoordinate::x_n = { 0,1,0,-1,1,1,-1,-1 };
	vector<int>FMTcoordinate::y_n = { 1,0,-1,0,1,-1,-1,1 };


    FMTcoordinate::FMTcoordinate():x(),y()/*,x_n({0,1,0,-1,1,1,-1,-1}),y_n({1,0,-1,0,1,-1,-1,1})*/{}
    FMTcoordinate::FMTcoordinate(unsigned int lx, unsigned int ly):
        x(lx),y(ly)/*,x_n({0,1,0,-1,1,1,-1,-1}),y_n({1,0,-1,0,1,-1,-1,1})*/{}
    FMTcoordinate::FMTcoordinate(const FMTcoordinate& rhs):x(rhs.x),y(rhs.y)/*,x_n(rhs.x_n),y_n(rhs.y_n)*/{}

    FMTcoordinate FMTcoordinate::at(unsigned int id) const
        {
        int factor = ((id / 8) + 1);
        id = (id - (factor-1) * 8);
        return FMTcoordinate(x+(x_n[id]*factor),y+(y_n[id]*factor));
        }
    double FMTcoordinate::distance(const FMTcoordinate& coord) const
        {
        //Should be
        int distancex = (x - coord.x);
        int distancey = (y - coord.y);
        return sqrt(distancex*distancex+distancey*distancey);
        //Was
        /*int distancex = (x - coord.x)^2;
        int distancey = (y - coord.y)^2;
        return sqrt(distancex - distancey);*/
        }
    bool FMTcoordinate::within(unsigned int ldistance,const FMTcoordinate& coord) const
        {
        int distancex = (x - coord.x);
        int distancey = (y - coord.y);
        if (static_cast<unsigned int>(abs(distancex))<=ldistance &&
			(static_cast<unsigned int>(abs(distancey))<=ldistance))
            {
            if (this->distance(coord) <= double(ldistance))
                {
                return true;
                }
            }
        return false;
        }
    unsigned int FMTcoordinate::getx() const
        {
        return x;
        }
    unsigned int FMTcoordinate::gety() const
        {
        return y;
        }
    FMTcoordinate& FMTcoordinate::operator = (const FMTcoordinate& rhs)
        {
        if(this!=&rhs)
            {
            x = rhs.x;
            y = rhs.y;
            /*x_n = rhs.x_n;
            y_n = rhs.y_n;*/
            }
        return *this;
        }
    bool FMTcoordinate::operator == (const FMTcoordinate& rhs) const
        {
        return (x == rhs.x && y == rhs.y);
        }
     bool FMTcoordinate::operator < (const FMTcoordinate& rhs) const
        {
        return ((y<rhs.y)||((y==rhs.y)&&(x<rhs.x)));
        }

    void FMTcoordinate::upenveloppe(vector<FMTcoordinate>& enveloppe) const
        {
		//0//-//1//
		//-//-//-//
		//2//-//3//
		int minx = min(enveloppe[0].x,x);
		int maxx = max(enveloppe[1].x,x);
		int miny = min(enveloppe[0].y,y);
		int maxy = max(enveloppe[2].y,y);
		enveloppe[0].x = minx;
		enveloppe[0].y = miny;
		enveloppe[1].x = maxx;
		enveloppe[1].y = miny;
		enveloppe[2].x = minx;
		enveloppe[2].y = maxy;
		enveloppe[3].x = maxx;
		enveloppe[3].y = maxy;
        }
    }

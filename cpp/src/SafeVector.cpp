#ifndef __SAFE_VEC_H__
#define __SAFE_VEC_H__
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <cstdio>
#include <algorithm>

namespace godot{

// poner 'sv' para escribirlo rápido
// A vector for error-prone humans
struct SafeVec
{
    std::int_fast32_t lef, RIGHT;

    SafeVec() {}
	SafeVec(int_fast32_t i, int_fast32_t j) : lef(i), RIGHT(j) {}

    SafeVec(const godot::Vector2i& vector2i) : lef(vector2i.x), RIGHT(vector2i.y){}

    bool operator==(const SafeVec& oSafeVec) const
    {return lef == oSafeVec.lef && RIGHT == oSafeVec.RIGHT;}
    bool operator!=(const SafeVec& oSafeVec) const {return !operator==(oSafeVec);}

    void operator+=(const SafeVec& oSafeVec) 
    {lef += oSafeVec.lef; RIGHT += oSafeVec.RIGHT;}
    void operator-=(const SafeVec& oSafeVec) 
    {lef -= oSafeVec.lef; RIGHT -= oSafeVec.RIGHT;}
    void operator*=(const SafeVec& oSafeVec) 
    {lef *= oSafeVec.lef; RIGHT *= oSafeVec.RIGHT;}
    void operator/=(const SafeVec& oSafeVec) 
    {lef /= oSafeVec.lef; RIGHT /= oSafeVec.RIGHT;}

    void addAssign_lef(const SafeVec& oSafeVec){lef += oSafeVec.lef;}
    void addAssignRIGHT(const SafeVec& oSafeVec){RIGHT += oSafeVec.RIGHT;}

    SafeVec sum_lef(const std::int_fast32_t i)const{return SafeVec(this->lef + i, RIGHT);}
    SafeVec sumRight(const std::int_fast32_t j)const{return SafeVec(lef, this->RIGHT + j);}

    SafeVec sum_lef(const SafeVec& oSafeVec)const{return SafeVec(lef + oSafeVec.lef, RIGHT);}
    SafeVec sumRIGHT(const SafeVec& oSafeVec)const{return SafeVec(lef, RIGHT + oSafeVec.RIGHT);}

    std::uint_fast8_t compare_lef(const SafeVec& oSafeVec) const
    {
        if (lef == oSafeVec.lef) 
            return 0;
        else if (lef < oSafeVec.lef)
            return -1;
        else 
            return 1;
    }
    std::uint_fast8_t compareRIGHT(const SafeVec& oSafeVec) const
    {
        if (RIGHT == oSafeVec.RIGHT) 
            return 0;
        else if (RIGHT < oSafeVec.RIGHT)
            return -1;
        else 
            return 1;
    }

    operator godot::Vector2i() const {return godot::Vector2i(lef, RIGHT);}
    operator godot::Vector2() const {return godot::Vector2(lef, RIGHT);}

    SafeVec operator+(const SafeVec& oSafeVec) const
    {return SafeVec(lef + oSafeVec.lef, RIGHT + oSafeVec.RIGHT);}
    SafeVec operator-(const SafeVec& oSafeVec) const
    {return SafeVec(lef - oSafeVec.lef, RIGHT - oSafeVec.RIGHT);}

    SafeVec operator-()const
    {return SafeVec(-lef, -RIGHT);}

    SafeVec operator*(const SafeVec& oSafeVec) const
    {return SafeVec(lef * oSafeVec.lef, RIGHT * oSafeVec.RIGHT);}
    SafeVec operator/(const SafeVec& oSafeVec) const
    {return SafeVec(lef / oSafeVec.lef, RIGHT / oSafeVec.RIGHT);}

    bool isStrictlySmallerThan(const SafeVec& oSafeVec) const {return lef < oSafeVec.lef && RIGHT < oSafeVec.RIGHT;}
    bool isStrictlyBiggerThan(const SafeVec& oSafeVec) const {return lef > oSafeVec.lef && RIGHT > oSafeVec.RIGHT;}

    bool operator<(const SafeVec& oSafeVec) const { return (lef == oSafeVec.lef) ? (RIGHT < oSafeVec.RIGHT) : (lef < oSafeVec.lef); }
	bool operator>(const SafeVec& oSafeVec) const { return (lef == oSafeVec.lef) ? (RIGHT > oSafeVec.RIGHT) : (lef > oSafeVec.lef); }

	bool operator<=(const SafeVec& oSafeVec) const { return lef == oSafeVec.lef ? (RIGHT <= oSafeVec.RIGHT) : (lef < oSafeVec.lef); }
	bool operator>=(const SafeVec& oSafeVec) const { return lef == oSafeVec.lef ? (RIGHT >= oSafeVec.RIGHT) : (lef > oSafeVec.lef); }

    bool isAnyCompNegative() const {return lef < 0 || RIGHT < 0;}

    bool isNonNegative() const{return lef >= 0 && RIGHT >= 0;}
    bool isStrictlyPositive() const{return lef > 0 && RIGHT > 0;}

    SafeVec operator*(double number) const
    {return SafeVec(lef * number, RIGHT * number);}
    SafeVec operator/(double number) const
    {return SafeVec(lef / number, RIGHT / number);}

    const char* c_str() const
    {
        static char buffer[28]; 
        snprintf(buffer, sizeof(buffer), "(%ld, %ld)", lef, RIGHT);
        return buffer;
    }

    double length() const{return distanceTo(SafeVec(0,0));}
    std::size_t area() const{return lef*RIGHT;}

    double distanceTo(const SafeVec& oSafeVec) const 
    {return std::sqrt((lef-oSafeVec.lef)*(lef-oSafeVec.lef) + (RIGHT-oSafeVec.RIGHT)*(RIGHT-oSafeVec.RIGHT));}

    double distanceSquaredTo(const SafeVec& oSafeVec) const 
    {return (lef - oSafeVec.lef)*(lef - oSafeVec.lef) + (RIGHT - oSafeVec.RIGHT)*(RIGHT - oSafeVec.RIGHT);}

    double distanceSquaredTo(const godot::Vector2i &oVector2i) const 
    {return (lef - oVector2i.x)*(lef - oVector2i.x) + (RIGHT - oVector2i.y)*(RIGHT - oVector2i.y);}

    struct hash
    {
        std::size_t operator()( const SafeVec& vec ) const
        {return vec.lef*31 + vec.RIGHT ;}
    };
};

}
#endif
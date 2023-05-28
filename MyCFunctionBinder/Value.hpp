#pragma once

#include <variant>
#include <string>

struct Value
{
   using Type = std::variant<int, bool, double, std::string, void*>;

   int getInt() const;
   bool getBool() const;
   double getDouble() const;
   const std::string & getString() const;
   void* getPointer() const;

   void setInt(int);
   void setBool(bool);
   void setDouble(double);
   void setString(const std::string&);
   void setPointer(void*);

   Type m_data;
};


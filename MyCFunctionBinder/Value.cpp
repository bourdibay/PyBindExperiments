#include "Value.hpp"

int Value::getInt() const
{
   return std::get<int>(m_data);
}

bool Value::getBool() const
{
   return std::get<bool>(m_data);
}

double Value::getDouble() const
{
   return std::get<double>(m_data);
}

const std::string& Value::getString() const
{
   return std::get<std::string>(m_data);
}

void* Value::getPointer() const
{
   return std::get<void*>(m_data);
}

void Value::setInt(int data)
{
   m_data = data;
}

void Value::setBool(bool data)
{
   m_data = data;
}

void Value::setDouble(double data)
{
   m_data = data;
}

void Value::setString(const std::string& data)
{
   m_data = data;
}

void Value::setPointer(void* data)
{
   m_data = data;
}


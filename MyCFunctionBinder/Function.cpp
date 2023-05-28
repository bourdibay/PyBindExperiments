#include "Function.hpp"

std::strong_ordering Function::operator<=>(const Function& rhs) const
{
   return std::tie(m_name, m_arity, m_argTypeList)
      <=> std::tie(rhs.m_name, rhs.m_arity, rhs.m_argTypeList);
}

bool Function::operator==(const Function& rhs) const
{
   return std::tie(m_name, m_arity, m_argTypeList)
      == std::tie(rhs.m_name, rhs.m_arity, rhs.m_argTypeList);
}

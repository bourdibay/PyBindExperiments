#include "ScriptEngine.hpp"

Function& ScriptEngine::declareFunctionOldSchool(
   const std::string& name,
   const OldSchoolFunction& callback,
   size_t nbArguments,
   const std::vector<Type>& arguments)
{
   Function function{ name, nbArguments, arguments, callback };

   const auto pos = std::lower_bound(m_functions.begin(),
      m_functions.end(),
      function);

   if (pos == m_functions.end() || *pos != function)
   {
      return *m_functions.insert(pos, std::move(function));
   }

   return *pos;
}

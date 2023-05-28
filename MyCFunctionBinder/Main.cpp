
#include "DataFrame.hpp"
#include "ScriptEngine.hpp"
#include "ScriptFunctionsBinding.hpp"
#include <iostream>

static int old_add(ScriptEngine* engine, DataFrame* frame)
{
   const auto& args = frame->m_argList;
   const int lhs = args[0].getInt();
   const int rhs = args[1].getInt();
   int res = lhs + rhs;

   if (args.size() > 2)
      res += args[2].getInt();

   frame->m_result.setInt(res);
   return 0;
}

static int new_add(int lhs, int rhs)
{
   return lhs + rhs;
}

static int new_add_3numbers(int nb1, int nb2, int nb3)
{
   return nb1 + nb2 + nb3;
}

static void executeAddFunction(ScriptEngine& engine,
   const Function& function,
   std::initializer_list<int> numbers)
{
   DataFrame frame;
   frame.m_argList.resize(numbers.size());

   int index = 0;
   for (int nb : numbers)
      frame.m_argList[index++].setInt(nb);

   function.m_callback(&engine, &frame);

   std::cout
      << "Return value of "
      << function.m_name
      << "(";

   index = 0;
   for (int nb : numbers)
   {
      std::cout << nb;
      if (++index < numbers.size())
         std::cout << ", ";
   }

   std::cout << ") is "
      << frame.m_result.getInt()
      << std::endl;
}

static int old_countFunctions(ScriptEngine* engine, DataFrame* frame)
{
   const auto& args = frame->m_argList;
   const auto str = args[0].getString();

   // access engine's data
   const size_t nbFunctions = engine->m_functions.size();
   const auto nbFunctionsString = std::to_string(nbFunctions);

   frame->m_result.setString(str + ": " + nbFunctionsString);
   return 0;
}

static std::string new_countFunctions(ScriptEngine* engine, std::string str)
{
   const size_t nbFunctions = engine->m_functions.size();
   return str + ": " + std::to_string(nbFunctions);
}

static void executeCountFunctionsFunction(ScriptEngine& engine,
   const Function& function,
   const std::string& str)
{
   DataFrame frame;
   frame.m_argList.resize(1);
   frame.m_argList[0].setString(str);

   function.m_callback(&engine, &frame);

   std::cout
      << "Return value of "
      << function.m_name
      << "("
      << str
      << ") is '"
      << frame.m_result.getString()
      << "'"
      << std::endl;
}

int main(int, char**)
{
   ScriptEngine engine;

   // old API
   {
      auto& oldAddFunction
         = engine.declareFunctionOldSchool(
            "old_add",
            old_add,
            2,
            { Type::Integer, Type::Integer, Type::Integer });

      executeAddFunction(engine, oldAddFunction, { 8, 12 });
      executeAddFunction(engine, oldAddFunction, { 8, 12, 50 });

      auto& oldCountFunction
         = engine.declareFunctionOldSchool(
            "old_count",
            old_countFunctions,
            1,
            { Type::String, Type::String });

      executeCountFunctionsFunction(engine, oldCountFunction, "Number of functions: ");
   }

   // new API
   {
      auto& newAddFunction = addTypedFunction<&new_add>(engine, "new_add");
      executeAddFunction(engine, newAddFunction, { 8, 12 });

      auto& newAddWith3NumbersFunction = addTypedFunction<&new_add_3numbers>(engine, "new_add");
      executeAddFunction(engine, newAddWith3NumbersFunction, { 8, 12, 50 });

      auto& newCountFunction = addTypedFunction<&new_countFunctions>(engine, "new_count");
      executeCountFunctionsFunction(engine, newCountFunction, "Number of functions: ");
   }

   return 0;
}

#pragma once

/*
* This is the wrapper built on top on the old C-style function
* that let us bind a function pointer to a function name.
*/
#include "ScriptEngine.fwd.hpp"
#include "ScriptEngine.hpp"
#include "DataFrame.hpp"
#include "Function.hpp"
#include "Value.hpp"
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace internal {

   template<typename T>
   struct always_false : std::false_type {};

   /*
    * Get the index of a type in a tuple
    */
   template <class T, class Tuple>
   struct IndexTuple
   {
      static_assert(!std::is_same_v<Tuple, std::tuple<>>, "Could not find `T` in given `Tuple`");
   };

   template <class T, class... Types>
   struct IndexTuple<T, std::tuple<T, Types...>>
   {
      static constexpr const std::size_t value = 0;
   };

   template <class T, class U, class... Types>
   struct IndexTuple<T, std::tuple<U, Types...>>
   {
      static constexpr const std::size_t value = 1 + IndexTuple<T, std::tuple<Types...>>::value;
   };

   /*
    * Helper: true if a tuple contains Type
    */
   template <typename T, typename Tuple>
   struct has_type;

   template <typename T, typename... Us>
   struct has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

   /*
    * Keep the argument types of a C++ function
    */
   template<typename... Args>
   struct FunctionArguments
   {
      using type = std::tuple<Args...>;

      static constexpr std::size_t size()
      {
         return std::tuple_size_v<FunctionArguments::type>;
      }
   };

   /*
    * Keep the argument types and the return type of a C++ function.
    * So far we handle function pointers and std::function.
    */
   template<typename T>
   struct FunctionInternal
   {
   };

   // Overload for std::function, or functors
   template<typename Ret, typename... Args>
   struct FunctionInternal<std::function<Ret(Args...)> >
   {
      using return_type = Ret;
      using arguments_type = FunctionArguments<Args...>;
   };

   // Overload for function pointers
   template<typename Ret, typename... Args>
   struct FunctionInternal<Ret(*)(Args...)>
   {
      using return_type = Ret;
      using arguments_type = FunctionArguments<Args...>;
   };

   /*
    * Converter C++ types to API types.
    */
   template<class T>
   struct FunctionType
   {
      static constexpr Type type()
      {
         static_assert(std::is_pointer_v<T>, "This C++ type is not implemented, use the legacy API");
         return Type::Pointer;
      }
   };

   template<>
   struct FunctionType<std::string>
   {
      static constexpr Type type()
      {
         return Type::String;
      }
   };

   template<>
   struct FunctionType<bool>
   {
      static constexpr Type type()
      {
         return Type::Boolean;
      }
   };

   template<>
   struct FunctionType<int>
   {
      static constexpr Type type()
      {
         return Type::Integer;
      }
   };

   template<>
   struct FunctionType<double>
   {
      static constexpr Type type()
      {
         return Type::Numeric;
      }
   };

   /*
    * Wrapper to retrieve the Value value correctly casted.
    */
   template<typename T>
   struct ValueRetriever
   {
      static T get(const Value& buffer)
      {
         static_assert(std::is_pointer_v<T>, "[Getter] This C++ type is not implemented, use the legacy API");

         return static_cast<T>(buffer.getPointer());
      }
   };

   template<>
   struct ValueRetriever<int>
   {
      static int get(const Value& buffer)
      {
         return buffer.getInt();
      }
   };

   template<>
   struct ValueRetriever<bool>
   {
      static bool get(const Value& buffer)
      {
         return buffer.getBool();
      }
   };

   template<>
   struct ValueRetriever<double>
   {
      static double get(const Value& buffer)
      {
         return buffer.getDouble();
      }
   };

   template<>
   struct ValueRetriever<std::string>
   {
      static std::string get(const Value& buffer)
      {
         return buffer.getString();
      }
   };


   template<>
   struct ValueRetriever<ScriptEngine*>
   {
      static ScriptEngine* get(const Value& buffer)
      {
         return static_cast<ScriptEngine*>(buffer.getPointer());
      }
   };

   /*
    * Wrapper to set the Value value correctly casted.
    */
   template<typename T>
   struct ValueSetter
   {
      static void set(Value& buffer, T value)
      {
         static_assert(std::is_pointer_v<T>, "[Setter] This C++ type is not implemented, use the legacy API");

         buffer.setPointer(value);
      }
   };

   template<>
   struct ValueSetter<std::string>
   {
      static void set(Value& buffer, std::string value)
      {
         buffer.setString(value);
      }
   };

   template<>
   struct ValueSetter<int>
   {
      static void set(Value& buffer, int value)
      {
         buffer.setInt(value);
      }
   };

   template<>
   struct ValueSetter<bool>
   {
      static void set(Value& buffer, bool value)
      {
         buffer.setBool(value);
      }
   };

   template<>
   struct ValueSetter<double>
   {
      static void set(Value& buffer, double value)
      {
         buffer.setDouble(value);
      }
   };

   /*
    * Convert vector<Value *> to tuple<> with typed values.
    */
   template<typename ArgumentsTypes,
      size_t ...Index>
   auto createTypedArguments(
      const std::vector<Value>& argList,
      std::index_sequence<Index...>)
   {
      return std::make_tuple(
         ValueRetriever<
         std::tuple_element_t<Index, ArgumentsTypes>>::get(
            argList[Index]) ...);
   }

   template<typename FunctionInternal>
   auto createTypedArguments(const std::vector<Value>& argList)
   {
      auto indexSequence
         = std::make_index_sequence<
         FunctionInternal::arguments_type::size()>();

      return createTypedArguments<typename FunctionInternal::arguments_type::type>(
         argList,
         indexSequence);
   }

   /*
    * Insertion of some special API data structures
    * that are accepted in function prototypes.
    */
   template<typename FunctionArguments,
      typename SpecialDataType>
   static std::unique_ptr<Value> insertSpecialData(
      Arguments& argList,
      void* specialData)
   {
      constexpr const auto indexVarArgs
         = IndexTuple<SpecialDataType, FunctionArguments>::value;

      auto buffer = std::make_unique<Value>();
      buffer->setPointer(specialData);

      argList.insert(argList.begin() + indexVarArgs, *buffer);

      return buffer;
   }

   template<typename FunctionArguments>
   static std::unique_ptr<Value> insertScriptEngine(
      Arguments& argList,
      ScriptEngine* engine)
   {
      return insertSpecialData<FunctionArguments, ScriptEngine*>(
         argList,
         engine);
   }

   template<typename FunctionArguments,
      typename SpecialDataType>
   constexpr static std::pair<bool, size_t> hasSpecialType()
   {
      if constexpr (has_type<SpecialDataType, FunctionArguments>::value)
      {
         constexpr const size_t index
            = IndexTuple<SpecialDataType, FunctionArguments>::value;

         return std::pair<bool, size_t>(true, index);
      }

      return std::pair<bool, size_t>(false, 0);
   }

   /*
    * Proxy of the function pointer that is called
    * by the script engine.
    *
    * The function pointer takes a Frame and the Engine.
    * It forwards the buffer's values to the C++ functions
    * and takes care of the set of the returned value to the returned Value.
    *
    */
   template<auto Functor>
   struct FunctionProxy
   {
      static int callback([[maybe_unused]] ScriptEngine* engine,
         DataFrame* frame)
      {
         using FunctorType = decltype(Functor);
         using Arguments = typename FunctionInternal<FunctorType>::arguments_type::type;

         auto& argList = frame->m_argList;

         /*
          * HACK: we inject a ScriptEngine * if it is present in the
          * prototype of the function.
          * To do so, we create a temporary Value that will contain
          * the engine in its buffer.
          */
         std::unique_ptr<Value> engineValue;

         if constexpr (has_type<ScriptEngine*, Arguments>::value)
         {
            engineValue = insertScriptEngine<Arguments>(argList, engine);
         }

         auto typedArguments = createTypedArguments<FunctionInternal<FunctorType>>(argList);

         auto error = std::apply(Functor, typedArguments);

         Value& result = frame->m_result;
         ValueSetter<decltype(error)>::set(result, error);

         return false;
      }
   };

   /*
    * Verify we have the expected sequence of special data types.
    * We must have:
    * - ScriptEngine * at index 0
    */
   template<typename FunctorType>
   static constexpr void checkSpecialDataTypesInCorrectOrder()
   {
      using ArgumentsType = typename FunctionInternal<FunctorType>::arguments_type;
      using Arguments = typename ArgumentsType::type;

      constexpr const auto pairScriptEngine = hasSpecialType<Arguments, ScriptEngine*>();
      if constexpr (pairScriptEngine.first)
      {
         static_assert(pairScriptEngine.second == 0,
            "`ScriptEngine *` must be the first function parameter");
      }
   }

   /*
    * Create the types array based on the C++ function arguments.
    */
   template<typename ArgumentsTypes,
      typename TypesToRemove,
      size_t ...Indexes>
   constexpr auto filterTuple(std::index_sequence<Indexes...>)
   {
      return std::tuple_cat(
         std::conditional_t<
         has_type<std::tuple_element_t<Indexes, ArgumentsTypes>, TypesToRemove>::value,
         std::tuple<>,
         std::tuple<std::tuple_element_t<Indexes, ArgumentsTypes>>> {} ...);
   }

   template<typename FunctionInternal,
      typename TypesToRemove>
   constexpr auto filterTuple()
   {
      constexpr auto N = FunctionInternal::arguments_type::size();
      using Indexes = std::make_index_sequence<N>;

      return filterTuple<
         typename FunctionInternal::arguments_type::type,
         TypesToRemove>(Indexes{});
   }

   template<typename ArgumentsTypes, size_t ...Indexes>
   constexpr auto createAPITypes(std::index_sequence<Indexes...>)
   {
      return std::vector{ FunctionType<std::tuple_element_t<Indexes, ArgumentsTypes>>::type() ... };
   }

   template<typename ArgumentsTypes>
   constexpr auto createAPITypes()
   {
      constexpr auto N = std::tuple_size_v<ArgumentsTypes>;
      using Indexes = std::make_index_sequence<N>;

      return createAPITypes<ArgumentsTypes>(Indexes{});
   }

} // internal namespace

template<auto Functor>
Function& addTypedFunction(
   ScriptEngine& engine,
   const std::string& name)
{
   using FunctorType = decltype(Functor);

   /*
    * Abort if the special data types are not in the expected order.
    */
   internal::checkSpecialDataTypesInCorrectOrder<FunctorType>();

   /*
    * We remove special types (ScriptEngine *) from the list of types
    * that we use to build the types.
    */
   using TypesToRemove = std::tuple<ScriptEngine*>;

   using FilteredTypes = decltype(internal::filterTuple<internal::FunctionInternal<FunctorType>, TypesToRemove>());

   /*
    * When there is 0 argument, createAPITypes does not compile
    */
   std::vector<Type> apiTypes;
   if constexpr (std::tuple_size_v<FilteredTypes> > 0)
      apiTypes = internal::createAPITypes<FilteredTypes>();

   // append the returned value to the signature
   apiTypes.insert(apiTypes.begin(),
      internal::FunctionType<typename internal::FunctionInternal<FunctorType>::return_type>::type());

   return engine.declareFunctionOldSchool(name,
      internal::FunctionProxy<Functor>::callback,
      apiTypes.size() - 1,
      apiTypes);
}

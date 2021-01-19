#ifndef TK_UTILS_H
#define TK_UTILS_H
#include "TKSystem.h"

TK_NAMESPACE_BEGIN

#define TKMAC_ASSERT(Expression)\
	{\
		 assert(Expression);\
	}

template< class T > FORCEINLINE T Align(const T Ptr, unsigned int Alignment)
{
	return (T)(((unsigned int)Ptr + Alignment - 1) & ~(Alignment - 1));

}
template< class T > FORCEINLINE T Align1(const T Ptr, unsigned int Alignment)
{
	return (T)((unsigned int)Ptr + Alignment - (Ptr & (Alignment - 1)));
}

template<typename T>
FORCEINLINE T ABS(T t)
{
	// 	if (t < 0)
	// 		return -t;
	return t < 0 ? -t : t;
	/*return t; */
}
template<typename T>
FORCEINLINE T Min(T t0, T t1)
{
	return t0 < t1 ? t0 : t1;
}
template<typename T>
FORCEINLINE T Max(T t0, T t1)
{
	return t0 > t1 ? t0 : t1;
}
template<typename T>
FORCEINLINE T Clamp(T Value, T Max, T Min)
{
	if (Value >= Max)
	{
		return Max;
	}
	if (Value <= Min)
	{
		return Min;
	}
	return Value;
}
template <class T>
FORCEINLINE void Swap(T &t1, T &t2)
{
	T temp;
	temp = t1;
	t1 = t2;
	t2 = temp;
}
#define BIT(i) (1 << i)


#define USE_STL_TYPE_TRAIT
#ifdef USE_STL_TYPE_TRAIT
#include <type_traits>
#endif

#ifdef USE_STL_TYPE_TRAIT
#define HAS_TRIVIAL_CONSTRUCTOR(T) std::is_trivially_constructible<T>::value
#define HAS_TRIVIAL_DESTRUCTOR(T) std::is_trivially_destructible<T>::value
#define HAS_TRIVIAL_ASSIGN(T) std::is_trivially_assignable<T>::value
#define HAS_TRIVIAL_COPY(T) std::is_trivially_copyable<T>::value
#define IS_POD(T) std::is_pod<T>::value
#define IS_ENUM(T) std::is_enum<T>::value
#define IS_EMPTY(T) std::is_empty<T>::value


/**
* TIsFloatType
*/
template<typename T> struct TIsFloatType { enum { Value = std::is_floating_point<T>::value }; };


/**
* TIsIntegralType
*/
template<typename T> struct TIsIntegralType { enum { Value = std::is_integral<T>::value }; };


/**
* TIsArithmeticType
*/
template<typename T> struct TIsArithmeticType
{
	enum { Value = std::is_arithmetic<T>::value };
};

/**
* TIsPointerType
* @todo - exclude member pointers
*/
template<typename T> struct TIsPointerType { enum { Value = std::is_pointer<T>::value }; };


/**
* TIsVoidType
*/
template<typename T> struct TIsVoidType { enum { Value = std::is_void<T>::value }; };


/**
* TIsPODType
*/
template<typename T> struct TIsPODType
{
	enum { Value = IS_POD(T) };
};

/**
* TIsFundamentalType
*/
template<typename T>
struct TIsFundamentalType
{
	enum { Value = std::is_fundamental<T>::Value };
};

template<typename T> struct ValueBase
{
	enum { NeedsConstructor = !HAS_TRIVIAL_CONSTRUCTOR(T) && !TIsPODType<T>::Value };
	enum { NeedsDestructor = !HAS_TRIVIAL_DESTRUCTOR(T) && !TIsPODType<T>::Value };
};
#else
#if _MSC_VER >= 1400
#define HAS_TRIVIAL_CONSTRUCTOR(T) __has_trivial_constructor(T)
#define HAS_TRIVIAL_DESTRUCTOR(T) __has_trivial_destructor(T)
#define HAS_TRIVIAL_ASSIGN(T) __has_trivial_assign(T)
#define HAS_TRIVIAL_COPY(T) __has_trivial_copy(T)
#define IS_POD(T) __is_pod(T)
#define IS_ENUM(T) __is_enum(T)
#define IS_EMPTY(T) __is_empty(T)
#else
#define HAS_TRIVIAL_CONSTRUCTOR(T) false
#define HAS_TRIVIAL_DESTRUCTOR(T) false
#define HAS_TRIVIAL_ASSIGN(T) false
#define HAS_TRIVIAL_COPY(T) false
#define IS_POD(T) false
#define IS_ENUM(T) false
#define IS_EMPTY(T) false
#endif


/*-----------------------------------------------------------------------------
Type traits similar to TR1 (uses intrinsics supported by VC8)
Should be updated/revisited/discarded when compiler support for tr1 catches up.
-----------------------------------------------------------------------------*/

/**
* TIsFloatType
*/
template<typename T> struct TIsFloatType { enum { Value = false }; };

template<> struct TIsFloatType<float> { enum { Value = true }; };
template<> struct TIsFloatType<double> { enum { Value = true }; };
template<> struct TIsFloatType<long double> { enum { Value = true }; };

/**
* TIsIntegralType
*/
template<typename T> struct TIsIntegralType { enum { Value = false }; };

template<> struct TIsIntegralType<unsigned char> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned short> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned int> { enum { Value = true }; };
template<> struct TIsIntegralType<unsigned long> { enum { Value = true }; };

template<> struct TIsIntegralType<signed char> { enum { Value = true }; };
template<> struct TIsIntegralType<signed short> { enum { Value = true }; };
template<> struct TIsIntegralType<signed int> { enum { Value = true }; };
template<> struct TIsIntegralType<signed long> { enum { Value = true }; };

template<> struct TIsIntegralType<bool> { enum { Value = true }; };
template<> struct TIsIntegralType<char> { enum { Value = true }; };

// compilers we support define wchar_t as a native type
#if !_MSC_VER || defined(_NATIVE_WCHAR_T_DEFINED)
template<> struct TIsIntegralType<wchar_t> { enum { Value = true }; };
#endif

// C99, but all compilers we use support it
template<> struct TIsIntegralType<unsigned long long> { enum { Value = true }; };
template<> struct TIsIntegralType<signed long long> { enum { Value = true }; };
/**
* TIsArithmeticType
*/
template<typename T> struct TIsArithmeticType
{
	enum { Value = TIsIntegralType<T>::Value || TIsFloatType<T>::Value };
};

/**
* TIsPointerType
* @todo - exclude member pointers
*/
template<typename T> struct TIsPointerType { enum { Value = false }; };
template<typename T> struct TIsPointerType<T*> { enum { Value = true }; };
template<typename T> struct TIsPointerType<const T*> { enum { Value = true }; };
template<typename T> struct TIsPointerType<const T* const> { enum { Value = true }; };
template<typename T> struct TIsPointerType<T* volatile> { enum { Value = true }; };
template<typename T> struct TIsPointerType<T* const volatile> { enum { Value = true }; };

/**
* TIsVoidType
*/
template<typename T> struct TIsVoidType { enum { Value = false }; };
template<> struct TIsVoidType<void> { enum { Value = true }; };
template<> struct TIsVoidType<void const> { enum { Value = true }; };
template<> struct TIsVoidType<void volatile> { enum { Value = true }; };
template<> struct TIsVoidType<void const volatile> { enum { Value = true }; };

/**
* TIsPODType
* @todo - POD array and member pointer detection
*/
template<typename T> struct TIsPODType
{
	enum { Value = IS_POD(T) || IS_ENUM(T) || TIsArithmeticType<T>::Value || TIsPointerType<T>::Value };
};

/**
* TIsFundamentalType
*/
template<typename T>
struct TIsFundamentalType
{
	enum { Value = TIsArithmeticType<T>::Value || TIsVoidType<T>::Value };
};


template<typename T> struct ValueBase
{
	// WRH - 2007/11/28 - the compilers we care about do not produce equivalently efficient code when manually
	// calling the constructors of trivial classes. In array cases, we can call a single memcpy
	// to initialize all the members, but the compiler will call memcpy for each element individually,
	// which is slower the more elements you have.
	enum { NeedsConstructor = !HAS_TRIVIAL_CONSTRUCTOR(T) && !TIsPODType<T>::Value };
	// WRH - 2007/11/28 - the compilers we care about correctly elide the destructor code on trivial classes
	// (effectively compiling down to nothing), so it is not strictly necessary that we have NeedsDestructor.
	// It doesn't hurt, though, and retains for us the ability to skip destructors on classes without trivial ones
	// if we should choose.
	enum { NeedsDestructor = !HAS_TRIVIAL_DESTRUCTOR(T) && !TIsPODType<T>::Value };
};
#endif

TK_NAMESPACE_END
#endif // !TK_UTILS_H
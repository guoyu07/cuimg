#ifndef CUIMG_GPU_BINARY_ADD_H_
# define CUIMG_GPU_BINARY_ADD_H_

# include <cuimg/meta.h>
# include <cuimg/dsl/binary_op.h>
# include <cuimg/dsl/traits.h>

namespace cuimg
{

  struct binary_add {};

  template <>
  struct evaluator<binary_add>
  {
    template <typename A, typename B>
      static inline __host__ __device__
      typename return_type<binary_op<binary_add, A, B, 0, 0> >::ret
      run(A& a, B& b, point2d<int> p)
    {
      return eval(a, p) + eval(b, p);
    }

  };

  template <typename A1, typename A2, unsigned P1, unsigned P2>
  struct return_type<binary_op<binary_add, A1, A2, P1, P2> >
  {
    typedef typename meta::type_add<typename return_type<A1>::ret,
                                    typename return_type<A2>::ret>::ret ret;
  };

  template <typename E, typename F>
  inline
  typename first<typename binary_op_<binary_add, E, F>::ret,
                 typename E::is_expr,
		 typename meta::equal<typename is_expr_default_void<F>::ret, void>::checktype >::ret
  operator+(const E& a, const F& b)
  {
    typedef typename binary_op_<binary_add, E, F>::ret return_type;
    return return_type(a, b);
  }


  template <typename E, typename F>
  inline
  typename first<typename binary_op_<binary_add, E, F>::ret,
                 typename F::is_expr,
		 typename meta::equal<typename is_expr_default_void<E>::ret, void>::checktype >::ret
  operator+(const E& a, const F& b)
  {
    typedef typename binary_op_<binary_add, E, F>::ret return_type;
    return return_type(a, b);
  }

  template <typename E, typename F>
  inline
  typename first<typename binary_op_<binary_add, E, F>::ret,
                 typename E::is_expr,
                 typename F::is_expr>::ret
  operator+(const E& a, const F& b)
  {
    typedef typename binary_op_<binary_add, E, F>::ret return_type;
    return return_type(a, b);
  }

}

#endif

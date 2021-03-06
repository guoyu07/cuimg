#ifndef CUIMG_GAUSSIAN_STATIC_H_
# define CUIMG_GAUSSIAN_STATIC_H_

# include <cuimg/util.h>
# include <cuimg/error.h>
# include <cuimg/meta_gaussian/meta_gaussian.h>
# include <cuimg/gpu/texture.h>
# include <cuimg/gpu/kernel_image2d.h>
# include <cuimg/gpu/device_image2d.h>

namespace cuimg
{
  namespace gaussian_internal
  {


    template <typename T>
    struct UNIT_STATIC(g_input_tex);
    REGISTER_TEXTURE2D_PROXY(g_input_tex);

    template <typename I, int R, int E, typename G>
      struct gaussian2d_row_loop
      {
        template <typename U>
        static __device__ inline U iter(const kernel_image2d<U>& out, const i_int2& p)
        {;
          return U(tex2D(UNIT_STATIC(g_input_tex)<typename U::cuda_bt>::tex(), p.y + R, p.x)) * meta_gaussian_coef<G, R>::coef() +
            gaussian2d_row_loop<I, R + 1, E, G>::iter(out, p);
        }
      };

    template <typename I, int E, typename G>
      struct gaussian2d_row_loop<I, E, E, G>
    {
      template <typename U>
        static __device__ inline U iter(const kernel_image2d<U>& out, const i_int2& p)
      {;
        return U(tex2D(UNIT_STATIC(g_input_tex)<I>::tex(), p.y + E, p.x)) * meta_gaussian_coef<G, E>::coef();
      }
    };

    template <typename I, typename O, typename G, int KERNEL_HALF_SIZE>
    static __global__ void gaussian_row_static_kernel(kernel_image2d<O> out)
    {
      int idr = blockIdx.y * blockDim.y + threadIdx.y;
      int idc = blockIdx.x * blockDim.x + threadIdx.x;
      i_int2 p(idr, idc);
      if (!out.has(p))
        return;
      out(p) = gaussian2d_row_loop<I, -KERNEL_HALF_SIZE, KERNEL_HALF_SIZE, G>::iter(out, p);
    }

    template <typename I, int R, int E, typename G>
      struct gaussian2d_col_loop
      {
        template <typename U>
        static __device__ inline U iter(const kernel_image2d<U>& out, const i_int2& p)
        {;
          return U(tex2D(UNIT_STATIC(g_input_tex)<typename U::cuda_bt>::tex(), p.y, p.x + R)) * meta_gaussian_coef<G, R>::coef() +
            gaussian2d_col_loop<I, R + 1, E, G>::iter(out, p);
        }
      };

    template <typename I, int E, typename G>
      struct gaussian2d_col_loop<I, E, E, G>
    {
      template <typename U>
        static __device__ inline U iter(const kernel_image2d<U>& out, const i_int2& p)
      {;
        return U(tex2D(UNIT_STATIC(g_input_tex)<I>::tex(), p.y, p.x + E)) * meta_gaussian_coef<G, E>::coef();
      }
    };

    template <typename I, typename O, typename G, int KERNEL_HALF_SIZE>
    static __global__ void gaussian_col_static_kernel(kernel_image2d<O> out)
    {
      int idr = blockIdx.y * blockDim.y + threadIdx.y;
      int idc = blockIdx.x * blockDim.x + threadIdx.x;
      i_int2 p(idr, idc);
      if (!out.has(p))
        return;
      out(p) = gaussian2d_col_loop<I, -KERNEL_HALF_SIZE, KERNEL_HALF_SIZE, G>::iter(out, p);
    }
  }

  template <typename I, typename O, typename G, int KERNEL_HALF_SIZE>
  void gaussian_static_row2d(const I& in, O& out, dim3 dimblock = dim3(16, 16))
  {
    assert(in.domain() == out.domain());
    bindTexture2d(in, gaussian_internal::UNIT_STATIC(g_input_tex)<typename I::value_type::cuda_bt>::tex());
    dim3 dimgrid = grid_dimension(in.domain(), dimblock);
    gaussian_internal::gaussian_row_static_kernel
    <typename I::value_type::cuda_bt, typename O::value_type, G, KERNEL_HALF_SIZE>
      <<<dimgrid, dimblock>>>(mki(out));
    cudaUnbindTexture(gaussian_internal::UNIT_STATIC(g_input_tex)<typename I::value_type::cuda_bt>::tex());
    check_cuda_error();
  }

  template <typename I, typename O, typename G, int KERNEL_HALF_SIZE>
  void gaussian_static_col2d(const I& in, O& out, dim3 dimblock = dim3(16, 16))
  {
    assert(in.domain() == out.domain());
    bindTexture2d(in, gaussian_internal::UNIT_STATIC(g_input_tex)<typename I::value_type::cuda_bt>::tex());
    dim3 dimgrid = grid_dimension(in.domain(), dimblock);
    gaussian_internal::gaussian_col_static_kernel
    <typename I::value_type::cuda_bt, typename O::value_type, G, KERNEL_HALF_SIZE>
      <<<dimgrid, dimblock>>>(mki(out));
    cudaUnbindTexture(gaussian_internal::UNIT_STATIC(g_input_tex)<typename I::value_type::cuda_bt>::tex());
    check_cuda_error();
  }

}

#endif

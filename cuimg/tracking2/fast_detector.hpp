#ifndef CUIMG_FAST_DETECTOR_HPP_
# define CUIMG_FAST_DETECTOR_HPP_

# include <cuimg/mt_apply.h>

namespace cuimg
{

  template <typename A>
  fast_detector<A>::fast_detector(const obox2d& d)
    : n_(9),
      saliency_(d),
      contrast_(d),
      new_points_(d)
  {
  }

  template <typename A>
  fast_detector&
  fast_detector<A>::set_fast_threshold(float f)
  {
    fast_th_ = f;
    return *this;
  }

  template <typename A>
  fast_detector&
  fast_detector<A>::set_n(unsigned n)
  {
    n_ = n;
    return *this;
  }


#ifndef NO_CPP0X
  template <typename A>
  void
  fast_detector<A>::update(const host_image2d<gl8u>& input)
  {
    input_ = input;

    mt_apply2d(sizeof(i_float1), input.domain() - border(8),
	       [this, &input] (i_int2 p)
	       {
		 saliency_(p) = fast::compute_saliency(p, input, n_, fast_th_);
	       }, arch::cpu());

    // contrast
    mt_apply2d(sizeof(i_float1), input.domain() - border(5),
	       [this, &input] (i_int2 p)
	       {
		 contrast_(p) = fast::compute_contrast(p, input, contrast_);
	       }, arch::cpu());

  }


  template <typename A>
  template <typename F, typename PS>
  void
  fast_detector<A>::new_particles(const F& feature, PS& pset)
  {
    SCOPE_PROF(fast_new_particles_detector);
    memset(new_points_, 0);
    mt_apply2d(sizeof(i_float1), saliency_.domain() - border(8),
               [this, &feature, &pset] (i_int2 p)
               {
                 if (pset.has(p)) return;
                 if (saliency_(p) == 0) return;

                 for (int i = 0; i < 8; i++)
                 {
                   i_int2 n(p + i_int2(c8[i]));
                   if (saliency_(p) < saliency_(n) || pset.has(n))
                     return;
                 }

                 new_points_(p) = 1;
               }, arch::cpu());

    st_apply2d(sizeof(i_float1), saliency_.domain() - border(8),
               [this, &feature, &pset] (i_int2 p)
               {
                 if (new_points_(p)) pset.add(p, feature(p));
               }, arch::cpu());

  }


#endif

#ifndef NO_CUDA

  template <typename PS, typename I, typename J>
  inline __global__
  void select_particles(PS& pset, const I& saliency, J& new_points)
  {
    i_int2 p = thread_pos2d();

    if (pset.has(p)) return;
    if (saliency_(p) == 0) return;

    for (int i = 0; i < 8; i++)
    {
      i_int2 n(p + i_int2(c8[i]));
      if (saliency_(p) < saliency_(n) || pset.has(n))
	return;
    }

    new_points_(p) = p;
  }


  template <typename A>
  template <typename F, typename PS>
  void
  fast_detector<A>::new_particles(const F& feature, PS& pset)
  {
    SCOPE_PROF(fast_new_particles_detector);
    memset(new_points_, 0);

    //dim3 dimblock = A::dimblock();
    //dim3 dimgrid = ;
    select_particles<<<A::dimgrid(saliency_.domain()), A::dimblock()>>>
      (pset, saliency_, new_points_);

    pset.append_new_points(new_points_);
  }


#endif

}

#endif

#pragma once
#include <datastorage.h>
namespace quiss
{
    template <typename T>
    inline auto distance(const MeridionalGridPoint<T> &gp1, const MeridionalGridPoint<T> &gp2) -> T
    {
        return sqrt((gp1.y - gp2.y) * (gp1.y - gp2.y) + (gp1.x - gp2.x) * (gp1.x - gp2.x));
    }

    template <typename T>
    inline auto compute_abscissas(MeridionalGrid<T> &g)
    {
        size_t ni = g.nRows();
        size_t nj = g.nCols();
        for (auto i = 0; i < ni; i++)
        {
            for (auto j = 0; j < nj; j++)
            {
                g(i, j).m = i == 0 ? 0. : g(i - 1, j).m + distance(g(i, j), g(i - 1, j));
                g(i, j).l = j == 0 ? 0. : g(i, j - 1).l + distance(g(i, j), g(i, j - 1));
            }
        }
    }


    auto fz = [](const auto &gp) { return gp.x; };

    auto fr = [](const auto &gp) { return gp.y; };

    auto fm = [](const auto &gp) { return gp.m; };
    
    auto fl = [](const auto &gp) { return gp.l; };
    
    auto fphi = [](const auto &gp) { return gp.phi; };

    template <typename T>
    inline auto compute_angles(MeridionalGrid<T> &g)
    {
        size_t ni = g.nRows();
        size_t nj = g.nCols();

        T drqdm, dzqdm, drqdl, dzqdl;
        for (auto i = 0; i < ni; i++)
        {
            for (auto j = 0; j < nj; j++)
            {
                // auto gam_ = -PI / 2 + PI * j / (nj - 1.);
                drqdm = D1_O2_i(g, i, j, fr, fm);
                dzqdm = D1_O2_i(g, i, j, fz, fm);
                g(i,j).phi = atan2(drqdm, dzqdm); // Stream line angle
                drqdl = D1_O2_j(g, i, j, fr, fl);
                dzqdl = D1_O2_j(g, i, j, fz, fl);
                g(i,j).gam = atan2(dzqdl, drqdl); // Span line angle
            }
        }
    }
    
    template <typename T>
    inline auto compute_curvature(MeridionalGrid<T> &g)
    {
        size_t ni = g.nRows()-1;
        size_t nj = g.nCols();

        for (auto i = 1; i < ni; i++)
        {
            for (auto j = 0; j < nj; j++)
            {
                g(i,j).cur = D1_O2_i(g, i, j, fphi, fm);// TODO check why Aungier put -DphiDm
            }
        }
        for (auto j = 0; j < nj; j++)
        {
            g(0, j).cur = 2. * g(1, j).cur - g(2, j).cur;
            g(ni, j).cur = 2. * g(ni - 1, j).cur - g(ni - 2, j).cur;
        }
    }

    template <typename T>
    inline auto compute_geom_values(MeridionalGrid<T> &g)
    {
        compute_abscissas(g);
        compute_angles(g);
        compute_curvature(g);
    }

    template <typename T>
    inline auto compute_angles(MeridionalGrid<T> &g,const Array2d<quiss::Grid2dMetricsPoint<T>> &g_metrics)
    {
        size_t ni = g.nRows();
        size_t nj = g.nCols();
        T d_ksi = 1. / (ni - 1.);
        T d_eth = 1. / (nj - 1.);

        T drqdm, dzqdm, drqdl, dzqdl;
        for (auto i = 0; i < ni; i++)
        {
            for (auto j = 0; j < nj; j++)
            {
                // auto gam_ = -PI / 2 + PI * j / (nj - 1.);
                // drqdm = D1_O2_i(g, i, j, fr, fm);
                // dzqdm = D1_O2_i(g, i, j, fz, fm);
                drqdm = D1_O2_dx1(g,g_metrics, i, j, d_ksi, d_eth, fr);
                dzqdm = D1_O2_dx1(g,g_metrics, i, j, d_ksi, d_eth, fz);
                g(i,j).phi = atan2(drqdm, dzqdm); // Stream line angle
                // drqdl = D1_O2_j(g, i, j, fr, fl);
                // dzqdl = D1_O2_j(g, i, j, fz, fl);
                drqdl = D1_O2_dx2(g,g_metrics, i, j, d_ksi, d_eth, fr);
                dzqdl = D1_O2_dx2(g,g_metrics, i, j, d_ksi, d_eth, fz);
                g(i,j).gam = atan2(dzqdl, drqdl); // Span line angle
                g(i,j).cgp = std::cos(g(i,j).gam+g(i,j).phi);
                g(i,j).sgp = std::sin(g(i,j).gam+g(i,j).phi);
            }
        }
    }
    
    template <typename T>
    inline auto compute_curvature(MeridionalGrid<T> &g,const Array2d<quiss::Grid2dMetricsPoint<T>> &g_metrics)
    {
        size_t ni = g.nRows();
        size_t nj = g.nCols();
        T d_ksi = 1. / (ni - 1.);
        T d_eth = 1. / (nj - 1.);

        for (auto i = 1; i < ni-1; i++)
        {
            for (auto j = 0; j < nj; j++)
            {
                // g(i,j).cur = D1_O2_i(g, i, j, fphi, fm);// TODO check why Aungier put -DphiDm
                g(i,j).cur = D1_O2_dx1(g,g_metrics, i, j, d_ksi, d_eth, fphi);// TODO check why Aungier put -DphiDm
            }
        }
        for (auto j = 0; j < nj; j++) // extrapolate on bounds
        {
            g(0, j).cur = 2. * g(1, j).cur - g(2, j).cur;
            g(ni-1, j).cur = 2. * g(ni - 2, j).cur - g(ni - 3, j).cur;
        }
    }

    template <typename T>
    inline auto compute_grid_metrics(MeridionalGrid<T> &g, Array2d<quiss::Grid2dMetricsPoint<T>> g_metrics, const auto &f_m, const auto &f_l)
    {
        compute_abscissas(g);
        compute_metrics(g,f_m,f_l,g_metrics);
        compute_angles(g,g_metrics);
        compute_curvature(g,g_metrics);
    }
} // namespace quiss
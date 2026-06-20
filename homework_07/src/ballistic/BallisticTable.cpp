#include <algorithm>
#include <fstream>
#include <vector>

#include "ballistic/BallisticTable.h"


struct GridPoint
{
    int iz, iv, im, id, il;
    double tz, tv, tm, td, tl;
};

std::pair<int,double> findInterval(const std::vector<float>& axis,
             double value)
{
    auto upper =
        std::upper_bound(axis.begin(), axis.end(), value);

    int i =
        static_cast<int>(upper - axis.begin()) - 1;

    i = std::clamp(i, 0, static_cast<int>(axis.size()) - 2);

    double t = (value - axis[i]) / (axis[i+1] - axis[i]);

    return {i,t};
}

std::pair<double,double> interpolate(
    const GridPoint& p,
    const BallisticTable& table)
{
    std::pair<double,double> result{0.0,0.0};

    for (int bz=0;bz<2;bz++)
    for (int bv=0;bv<2;bv++)
    for (int bm=0;bm<2;bm++)
    for (int bd=0;bd<2;bd++)
    for (int bl=0;bl<2;bl++)
    {
        double weight =
            (bz ? p.tz : 1.0 - p.tz) *
            (bv ? p.tv : 1.0 - p.tv) *
            (bm ? p.tm : 1.0 - p.tm) *
            (bd ? p.td : 1.0 - p.td) *
            (bl ? p.tl : 1.0 - p.tl);

        const auto& node =
            table.at(
                p.iz + bz,
                p.iv + bv,
                p.im + bm,
                p.id + bd,
                p.il + bl);

        result.first  += node.t      * weight;
        result.second += node.hDist * weight;
    }

    return result;
}

std::pair<double,double> solve(
    const BallisticTable& table,
    double z, double v, double m, double d, double l)
{
    auto [iz,tz] = findInterval(table.axisZ0, z);
    auto [iv,tv] = findInterval(table.axisV0, v);
    auto [im,tm] = findInterval(table.axisM, m);
    auto [id,td] = findInterval(table.axisD, d);
    auto [il,tl] = findInterval(table.axisL, l);

    GridPoint p{
        iz,iv,im,id,il,
        tz,tv,tm,td,tl
    };

    return interpolate(p, table);
}

// Індекс у плоскому масиві: [iZ0][iV0][iM][iD][iL]
size_t BallisticTable::index(int iz, int iv, int im, int id, int il) const {
    return ((((size_t)iz * axisV0.size() + iv)
                            * axisM.size()  + im)
                            * axisD.size()  + id)
                            * axisL.size()  + il;
}

const BallisticTable::Result& BallisticTable::at(int iz, int iv, int im,
                    int id, int il) const {
    return data[index(iz, iv, im, id, il)];
}

bool BallisticTable::load(const char* path) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    int nZ, nV, nM, nD, nL;
    f >> nZ >> nV >> nM >> nD >> nL;

    axisZ0.resize(nZ); for (auto& v : axisZ0) f >> v;
    axisV0.resize(nV); for (auto& v : axisV0) f >> v;
    axisM.resize(nM);  for (auto& v : axisM)  f >> v;
    axisD.resize(nD);  for (auto& v : axisD)  f >> v;
    axisL.resize(nL);  for (auto& v : axisL)  f >> v;

    size_t total = (size_t)nZ*nV*nM*nD*nL;
    data.resize(total);

    // Порядок: Z0 → V0 → m → d → l (зовнішній → внутрішній)
    for (size_t i = 0; i < total; i++)
        f >> data[i].t >> data[i].hDist;

    return f.good();
}
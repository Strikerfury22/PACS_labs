// readable smallpt, a Path Tracer by Kevin Beason, 2008.  Adjusted
// for my particular readability sensitivities by Roger Allen, 2016
// Added C++11 multithreading & removed openmp.

// Make:
// smallpt_thd: smallpt_thd.cpp
//	g++ -Wall -std=c++11 -O3 smallpt_thd.cpp -o smallpt_thd
// thought uname -s 16384 would help, but it didn't.

// Usage: time ./smallpt 100
// N  real
// 1  151
// 2   81
// 3
// 4   43
// 5   40
// 6   40
// 7
// 8   32

#include <atomic>
#include <cmath>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

#include <thread_pool.hpp>

// Vec is a structure to store position (x,y,z) and color (r,g,b)
struct Vec {
    double x, y, z;
    explicit Vec(double x_=0.0, double y_=0.0, double z_=0.0){ x=x_; y=y_; z=z_; }
    Vec operator+(const Vec &b) const {
        return Vec(x+b.x,y+b.y,z+b.z);
    }
    Vec operator-(const Vec &b) const {
        return Vec(x-b.x,y-b.y,z-b.z);
    }
    Vec operator*(double b) const {
        return Vec(x*b,y*b,z*b);
    }
    Vec mult(const Vec &b) const {
        return Vec(x*b.x,y*b.y,z*b.z);
    }
    Vec& norm() {
        return *this = *this * (1/sqrt(x*x+y*y+z*z));
    }
    double dot(const Vec &b) const {
        return x*b.x+y*b.y+z*b.z;
    }
    // % is cross product
    // rallen ??? why no const here?
    Vec operator%(Vec&b){
        return Vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);
    }
};

struct Ray {
    Vec o, d;
    Ray(Vec o_, Vec d_) : o(o_), d(d_) {}
};

// material types, used in radiance()
enum Refl_t { DIFF, SPEC, REFR };

struct Sphere {
    double rad;       // radius
    Vec p, e, c;      // position, emission, color
    Refl_t refl;      // reflection type (DIFFuse, SPECular, REFRactive)
    Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_):
        rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}
    double intersect(const Ray &r) const {
        // returns distance, 0 if nohit
        Vec op = p-r.o; // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, eps=1e-4, b=op.dot(r.d), det=b*b-op.dot(op)+rad*rad;
        if (det<0) {
            return 0;
        }
        else {
            det=sqrt(det);
        }
        return (t=b-det)>eps ? t : ((t=b+det)>eps ? t : 0);
    }
};

//Scene: radius, position, emission, color, material
Sphere spheres[] = {
    Sphere(1e5, Vec( 1e5+1,40.8,81.6), Vec(),Vec(.75,.25,.25),DIFF),//Left
    Sphere(1e5, Vec(-1e5+99,40.8,81.6),Vec(),Vec(.25,.25,.75),DIFF),//Rght
    Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75),DIFF),//Back
    Sphere(1e5, Vec(50,40.8,-1e5+170), Vec(),Vec(),           DIFF),//Frnt
    Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75),DIFF),//Botm
    Sphere(1e5, Vec(50,-1e5+81.6,81.6),Vec(),Vec(.75,.75,.75),DIFF),//Top
    Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC),//Mirr
    Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR),//Glas
    Sphere(600, Vec(50,681.6-.27,81.6),Vec(12,12,12),  Vec(), DIFF) //Lite
};

inline double clamp(double x){ return x<0 ? 0 : x>1 ? 1 : x; }

inline int toInt(double x){ return int(pow(clamp(x),1/2.2)*255+.5); }

inline bool intersect(const Ray &r, double &t, int &id) {
    double n=sizeof(spheres)/sizeof(Sphere), d, inf=t=1e20;
    for(int i=int(n);i--;) {
        if((d=spheres[i].intersect(r))&&d<t){
            t=d;
            id=i;
        }
    }
    return t<inf;
}

std::atomic<int> max_depth{0};

Vec radiance(const Ray &r, int depth, unsigned short *Xi){
    double t;                               // distance to intersection
    int id=0;                               // id of intersected object
    if (!intersect(r, t, id)) {
        return Vec();  // if miss, return black
    }
    const Sphere &obj = spheres[id];        // the hit object
    Vec x=r.o+r.d*t, n=(x-obj.p).norm(), nl=n.dot(r.d)<0?n:n*-1, f=obj.c;
    double p = f.x>f.y && f.x>f.z ? f.x : f.y>f.z ? f.y : f.z; // max refl
    if (++depth>5) {
        if (erand48(Xi)<p){
            f=f*(1/p);
        }
        else{
            return obj.e; //R.R.
        }
    }
#if 1
    // RALLEN this isn't a good CAS, but is "good enough"
    if(depth > max_depth) {
        max_depth = depth;
    }
#endif
    if (obj.refl == DIFF) {
        // Ideal DIFFUSE reflection
        double r1=2*M_PI*erand48(Xi), r2=erand48(Xi), r2s=sqrt(r2);
        Vec w=nl, u=((fabs(w.x)>.1?Vec(0,1):Vec(1))%w).norm(), v=w%u;
        Vec d = (u*cos(r1)*r2s + v*sin(r1)*r2s + w*sqrt(1-r2)).norm();
        return obj.e + f.mult(radiance(Ray(x,d),depth,Xi));
    } else if (obj.refl == SPEC) {
        // Ideal SPECULAR reflection
        return obj.e + f.mult(radiance(Ray(x,r.d-n*2*n.dot(r.d)),depth,Xi));
    }
    Ray reflRay(x, r.d-n*2*n.dot(r.d));     // Ideal dielectric REFRACTION
    bool into = n.dot(nl)>0;                // Ray from outside going in?
    double nc=1, nt=1.5, nnt=into?nc/nt:nt/nc, ddn=r.d.dot(nl), cos2t;
    if ((cos2t=1-nnt*nnt*(1-ddn*ddn))<0) {    // Total internal reflection
        return obj.e + f.mult(radiance(reflRay,depth,Xi));
    }
    Vec tdir = (r.d*nnt - n*((into?1:-1)*(ddn*nnt+sqrt(cos2t)))).norm();
    double a=nt-nc, b=nt+nc, R0=a*a/(b*b), c = 1-(into?-ddn:tdir.dot(n));
    double Re=R0+(1-R0)*c*c*c*c*c,Tr=1-Re,P=.25+.5*Re,RP=Re/P,TP=Tr/(1-P);
    return obj.e + f.mult(depth>2 ? (erand48(Xi)<P ?   // Russian roulette
                                     radiance(reflRay,depth,Xi)*RP:radiance(Ray(x,tdir),depth,Xi)*TP) :
                          radiance(reflRay,depth,Xi)*Re+radiance(Ray(x,tdir),depth,Xi)*Tr);
}


struct Region {
    int x0, x1, y0, y1;
    explicit Region(int x0, int x1, int y0, int y1) :
        x0(x0), x1(x1), y0(y0), y1(y1) {}
    void print() {
      std::cout << "x0: " << x0 << ", x1: " << x1 << std::endl;
      std::cout << "y0: " << y0 << ", y1: " << y1 << std::endl;
      std::cout << std::endl;
    }
};

void render(int w, int h, int samps, Ray cam,
            Vec cx, Vec cy, Vec *c,
            const Region reg
    ) {
    int y0 = reg.y0, y1 = reg.y1;
    int x0 = reg.x0, x1 = reg.x1;

    for (int y=y0; y<y1; y++) {                       // Loop over image rows
        for (unsigned short x=x0, Xi[3]={0,0,static_cast<unsigned short>(y*y*y)}; x<x1; x++) {   // Loop cols
            for (int sy=0, i=(h-y-1)*w+x; sy<2; sy++) {     // 2x2 subpixel rows
                for (int sx=0; sx<2; sx++) {        // 2x2 subpixel cols
                    Vec r{0.0, 0.0, 0.0};
                    for (int s=0; s<samps; s++) {
                        double r1=2*erand48(Xi), dx=r1<1 ? sqrt(r1)-1: 1-sqrt(2-r1);
                        double r2=2*erand48(Xi), dy=r2<1 ? sqrt(r2)-1: 1-sqrt(2-r2);
                        Vec d = cx*( ( (sx+.5 + dx)/2 + x)/w - .5) +
                                                       cy*( ( (sy+.5 + dy)/2 + y)/h - .5) + cam.d;
                        r = r + radiance(Ray(cam.o+d*140,d.norm()),0,Xi)*(1./samps);
                    } // Camera rays are pushed ^^^^^ forward to start in interior
                    c[i] = c[i] + Vec(clamp(r.x),clamp(r.y),clamp(r.z))*.25;
                }
            }
        }
    }
}

//ANADIDO
// Una task que ejecuta una fila entera.
/*void multiRender(int w, int h, int samps, Ray cam,
            Vec cx, Vec cy, Vec *c, int row){
    for (int i = 0; i < w; i++){ //Iteramos sobre las columnas
        Region reg(i, i+1, row, row+1);
        render(w,h,samps,cam,cx,cy,c,reg);
    }

}*/
//FIN ANADIDO

std::pair<size_t, size_t>
usage(int argc, char *argv[], size_t w, size_t h) {
    // read the number of divisions from the command line
    if (!((argc == 1) || (argc == 3))) {
        std::cerr << "Invalid syntax: smallpt_thread_pool <width_divisions> <height_divisions>" << std::endl;
        exit(1);
    }

    size_t w_div = argc == 1 ? 2 : std::stol(argv[1]);
    size_t h_div = argc == 1 ? 2 : std::stol(argv[2]);

    if (((w/w_div) < 1) || ((h/h_div) < 1)){
        std::cerr << "The minimum region width and height is 1" << std::endl; //we've changed this so we can test with rows and columns. Originally 4
        exit(1);
    }
    return std::make_pair(w_div, h_div);
}

void write_output_file(const std::unique_ptr<Vec[]>& c, size_t w, size_t h)
{
    std::ofstream ofile("image3.ppm", std::ios::out);
    ofile << "P3" << std::endl;
    ofile << w << " " << h << std::endl;
    ofile << "255" << std::endl;
    for (size_t i=0; i<w*h; i++) {
      ofile << toInt(c[i].x) << " " << toInt(c[i].y) << " " << toInt(c[i].z) << std::endl;
    }
}

int main(int argc, char *argv[]){
    size_t w=1024, h=768, samps = 4; // # samples
    //std::array<int, 11> w_divisors = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    //std::array<int, 18> h_divisors = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 768};
    
    Ray cam(Vec(50,52,295.6), Vec(0,-0.042612,-1).norm()); // cam pos, dir
    Vec cx=Vec(w*.5135/h), cy=(cx%cam.d).norm()*.5135;
    std::unique_ptr<Vec[]> c{new Vec[w*h]};
    
    auto p = usage(argc, argv, w, h);
    auto w_div = p.first;
    auto h_div = p.second;
    const auto y_height = h / h_div;
    const auto x_width = w / w_div;
    auto start = std::chrono::steady_clock::now();
    auto *c_ptr = c.get(); // raw pointer to Vector c
    std::cout << "w_div " << w_div << " h_div " << h_div << " y_height " << y_height << " x_width " << x_width << std::endl;
    {
        // create a thread pool
        thread_pool our_pool;
        // launch the tasks
        /*Pixel by pixel*/
        for (size_t i = 0; i < w_div; ++i){
            for (size_t j = 0; j < h_div; ++j){
                size_t y0 = i * y_height; 
                size_t y1 = i == h_div - 1 ? h : y0 + y_height;
                size_t x0 = j * x_width;    
                size_t x1 = j == w_div - 1 ? w : x0 + x_width;
                std::cout << "x0 " << x0 << " x1 " << x1 << " y0 " << y0 << " y1 " << y1 << std::endl;
                Region reg(x0, x1, y0, y1);
                our_pool.submit([=] {render(w,h,samps,cam,cx,cy,c_ptr,reg); });
                //render(w,h,samps,cam,cx,cy,c_ptr,reg);
            }
        } 
        //our_pool.wait();
        /*Row by Row*/
        /*for (size_t j = 0; j < h; j++){
            Region reg(0, w, j, j+1);
            render(w,h,samps,cam,cx,cy,c_ptr,reg);
        }*/
        /*Column by Column*/
        /*for (size_t i = 0; i < w; i++){
            Region reg(i, i+1, 0, h);
            render(w,h,samps,cam,cx,cy,c_ptr,reg);
        }*/
        /*Square*/
        //mcd (1024, 768) = 256
        /*size_t square_shape = 256;
        for (size_t i = 0; i < w; i += square_shape){
            for (size_t j = 0; j < h; j += square_shape){
                Region reg(i, i+square_shape, j, j+square_shape);
                render(w,h,samps,cam,cx,cy,c_ptr,reg);
            }
        }*/
        /*Test all sizes*/
        /*
        std::vector<std::pair<double, std::pair<int, int>>> results;
        for (size_t i = 0; i < w_divisors.size(); i++){
            for (size_t j = 0; j < h_divisors.size(); j++){
                auto start = std::chrono::steady_clock::now();
                for (size_t k = 0; k < w; k += w_divisors[i]){
                    for (size_t l = 0; l < h; l += h_divisors[j]){
                    Region reg(k, k + w_divisors[i], l, l + h_divisors[j]);
                    render(w,h,samps,cam,cx,cy,c_ptr,reg);
                }
                auto stop = std::chrono::steady_clock::now();
                auto duration =  std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count();
                results.push_back({duration, {w_divisors[i], h_divisors[j]}});
                std::cout << "Iteration processed";
            }
            }
        } 
        std::sort(results.begin(), results.end(), [](const std::pair<double, std::pair<int, int>>& a, const std::pair<double, std::pair<int, int>>& b) {
                return a.first < b.first;
            });
        
        std::cout << "Resultados ordered by time:\n";
        for (const auto& result : results) {
            std::cout << "Time: " << result.first
                << ", Elements: (" << result.second.first << ", " << result.second.second << ")\n";
        }
        */
    }
    auto stop = std::chrono::steady_clock::now();
    std::cout << "Execution time: " <<
        std::chrono::duration_cast<std::chrono::milliseconds>(stop-start).count() << " ms." << std::endl;
    // wait for completion
    write_output_file(c, w, h);
}

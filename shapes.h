#ifndef SHAPES_H
#define SHAPES_H

#include "raytracing.h"
#include "hittable.h"

class sphere : public hittable {
  public:
     // Stationary Sphere
    sphere(const point3& static_center, double radius, shared_ptr<material> mat)
      : center(static_center, vec3(0,0,0)), radius(std::fmax(0,radius)), mat(mat)
    {
        auto rvec = vec3(radius, radius, radius);
        bbox = aabb(static_center - rvec, static_center + rvec);
    }

    // Moving Sphere
    sphere(const point3& center1, const point3& center2, double radius,
           shared_ptr<material> mat)
      : center(center1, center2 - center1), radius(std::fmax(0,radius)), mat(mat) 
      {
        auto rvec = vec3(radius, radius, radius);
        aabb box1(center.at(0) - rvec, center.at(0) + rvec);
        aabb box2(center.at(1) - rvec, center.at(1) + rvec);
        bbox = aabb(box1, box2);
      }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {        
        point3 current_center = center.at(r.time());
        vec3 oc = current_center - r.origin();
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h - a*c;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (h - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (h + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - current_center) / radius;        
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);
        rec.mat = mat;

        return true;
    }

    static void get_sphere_uv(const point3& p, double& u, double& v) {
        auto theta = std::acos(-p.y());
        auto phi = std::atan2(-p.z(), p.x()) + pi;

        u = phi / (2*pi);
        v = theta / pi;
    }

    aabb bounding_box() const override { return bbox; }

  private:
    ray center;
    double radius;
    shared_ptr<material> mat;
    aabb bbox;
};

class cube : public hittable {
  public:
     cube(const point3& center, double side_length, shared_ptr<material> mat)
      : center(center), side_length(std::fmax(0,side_length)), mat(mat) {}
      
      bool hit(const ray& r, interval ray_t, hit_record& rec) const override {        
        // Axis-aligned bounding box hit test
        vec3 min = center - vec3(side_length/2, side_length/2, side_length/2);
        vec3 max = center + vec3(side_length/2, side_length/2, side_length/2);

        double t_min = (min.x() - r.origin().x()) / r.direction().x();
        double t_max = (max.x() - r.origin().x()) / r.direction().x();
        if (t_min > t_max) std::swap(t_min, t_max);

        double ty_min = (min.y() - r.origin().y()) / r.direction().y();
        double ty_max = (max.y() - r.origin().y()) / r.direction().y();
        if (ty_min > ty_max) std::swap(ty_min, ty_max);

        if ((t_min > ty_max) || (ty_min > t_max))
            return false;

        if (ty_min > t_min)
            t_min = ty_min;
        if (ty_max < t_max)
            t_max = ty_max;

        double tz_min = (min.z() - r.origin().z()) / r.direction().z();
        double tz_max = (max.z() - r.origin().z()) / r.direction().z();
        if (tz_min > tz_max) std::swap(tz_min, tz_max);

        if ((t_min > tz_max) || (tz_min > t_max))
            return false;

        if (tz_min > t_min)
            t_min = tz_min;
        if (tz_max < t_max)
            t_max = tz_max;

        if (!ray_t.surrounds(t_min))
            return false;

        rec.t = t_min;
        rec.p = r.at(rec.t);
        
        // Determine the normal vector based on which face was hit
        vec3 outward_normal;
        if (std::fabs(rec.p.x() - min.x()) < 1e-8) {
            outward_normal = vec3(-1, 0, 0);
        } else if (std::fabs(rec.p.x() - max.x()) < 1e-8) {
            outward_normal = vec3(1, 0, 0);
        } else if (std::fabs(rec.p.y() - min.y()) < 1e-8) {
            outward_normal = vec3(0, -1, 0);
        } else if (std::fabs(rec.p.y() - max.y()) < 1e-8) {
            outward_normal = vec3(0, 1, 0);
        } else if (std::fabs(rec.p.z() - min.z()) < 1e-8) {
            outward_normal = vec3(0, 0, -1);
        } else if (std::fabs(rec.p.z() - max.z()) < 1e-8) {
            outward_normal = vec3(0, 0, 1);
        } else {
            return false; // Should not happen, but just in case
        }
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 center;
    double side_length;
    shared_ptr<material> mat;
};

#endif
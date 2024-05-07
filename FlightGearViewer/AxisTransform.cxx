/* ------------------------------------------------------------------   */
/*      item            : AxisTransform.cxx
        made by         : rvanpaassen
        date            : Mon Jun 29 12:58:26 2009
        category        : header file
        description     : convert aircraft axes to Ogre objects
        changes         : Mon Jun 29 12:58:26 2009 first version
        language        : C++
*/

#include "AxisTransform.hxx"
// #include "comm-objects.h"
#include <dueca/extra/RvPQuat.hxx>
#include <cmath>
#include <iostream>

#define FINAL_ALTITUDE_CORRECTION

#ifdef DEBUG_AXIS2
#define DEB(A) A
#else
#define DEB(A)
#endif

/** Elliptic world definition WGS 84 geoid */
static const double a = 6378137.0;
static const double f = 1 / 298.257223563;

// some default conversions and helper code
template <class T> inline T deg2rad(const T &A) { return M_PI / 180.0 * (A); }
template <class T> inline T rad2deg(const T &A) { return 180.0 / M_PI * (A); }
template <class T> inline T m2ft(const T &A) { return (1.0 / 0.3048) * (A); }
template <class T> inline T sqr(const T &x) { return x * x; }
template <class T> inline T sqr3(T x) { return x * x * x; }

FGAxis::FGAxis()
{
  //
}

FGAxis::~FGAxis()
{
  //
}

FGLocalAxis::FGLocalAxis(double lat_zero, double lon_zero, double h_zero,
                         double psi_zero) :
  psi_zero(psi_zero),
  axis(LatLonAlt(lat_zero, lon_zero, h_zero), psi_zero),
  q_psi0(EulerAngles(0.0, 0.0, deg2rad(-psi_zero)))
{
  //
}

void FGLocalAxis::transform(double result[6], const double xyz[3],
                            const double quat[4])
{
  // position and location
  Carthesian c_xyz(xyz[0], xyz[1], xyz[2]);

  // ecef coordinates from the axis and the relative coordinates
  ECEF ecef = axis.toECEF(c_xyz);

  // lat-lon-alt representation
  LatLonAlt lla(ecef);

  // fill in position
  result[0] = rad2deg(lla.lat);
  result[1] = rad2deg(lla.lon);
  result[2] = m2ft(lla.alt);

  // transform heading
  double qnew_data[4];
  VectorE qnew(qnew_data, 4);
  QxQ(qnew, q_psi0, quat);
  result[3] = rad2deg(Q2phi(qnew));
  result[4] = rad2deg(Q2tht(qnew));
  result[5] = rad2deg(Q2psi(qnew));
}

void FGLocalAxis::toECEF(double pos[3], float velocity[3], float attitude[4],
                         float omega[3], const double xyz[3],
                         const double quat[4], const float uvw[3],
                         const float pqr[3]) const
{
  // position and location
  Carthesian c_xyz(xyz[0], xyz[1], xyz[2]);

  // ecef coordinates from the local axis and the relative coordinates
  ECEF ecef = axis.toECEF(c_xyz);
  //  print_vector(ecef.xyz);
  VectorE v_pos(pos, 3);
  v_pos = ecef.xyz;

  // std::cerr << v_pos << std::endl;

  // lat-lon-alt representation of this location as step for the
  // orientation
  LatLonAlt lla(ecef);

  // Orientation conversion from the local north-heading lat, lon, alt
  // to the global ECEF system
  Orientation to_global = lla.toGlobal(psi_zero);

  // conversion of the quaternion is simply multiplication - order??
  QxQ(attitude, quat, to_global);

  printPTP(std::cout, quat);
  printPTP(std::cout, attitude);



  // Attitude forms the basis for the rotation matrix for converting
  // the speeds and rotational rates
  float R_data[9];
  MatrixfE R(R_data, 3, 3);
  Q2R(R, attitude);

  VectorfE v_velocity(velocity, 3);
  cVectorfE v_uvw(uvw, 3);
  VectorfE v_omega(omega, 3);
  cVectorfE v_pqr(pqr, 3);

  // now convert the attitude vector to axis-angle representation
  // https://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation
  float angle = 2.0*std::acos(attitude[0]);
  float norm = std::sqrt(attitude[1]*attitude[1] + attitude[2]*attitude[2] +
      attitude[3]*attitude[3]);
  if (std::fabsf(angle) < 1e-5f) {
    attitude[1] = 2.0f*attitude[1];
    attitude[2] = 2.0f*attitude[2];
    attitude[3] = 2.0f*attitude[3];
  }
  else {
    attitude[1] = attitude[1]/norm*angle;
    attitude[2] = attitude[2]/norm*angle;
    attitude[3] = attitude[3]/norm*angle;
  }


  // transform the speed vector, in body axes, to ECEF coordinates
  v_velocity = R * v_uvw;

  // transform the rotational speed vector, in body axes, to ECEF coordinates
  v_omega = R * v_pqr;
}

FGECEFAxis::FGECEFAxis()
{
  //
}

void FGECEFAxis::transform(double result[6], const double xyz[3],
                           const double quat[4])
{
  ECEF ecef(xyz[0], xyz[1], xyz[2]);

  // lat-lon-alt representation
  LatLonAlt lla(ecef);

    // fill in position
  result[0] = rad2deg(lla.lon);
  result[1] = rad2deg(lla.lat);
  result[2] = m2ft(lla.alt);

  // Need attitude with respect to local north ref axis
  // this gives the to_global orientation, need the reverse
  Orientation to_local = lla.toGlobal();
  to_local.L = -to_local.L;

  // convert the orientation quat to local
  double qres[4];
  QxQ(qres, to_local, quat);

  result[3] = rad2deg(Q2phi(qres)); // roll
  result[4] = rad2deg(Q2tht(qres)); // pitch
  result[5] = rad2deg(Q2psi(qres)); // yaw
}

void FGECEFAxis::toECEF(double pos[3], float velocity[3], float attitude[4],
                        float omega[3], const double xyz[3],
                        const double quat[4], const float uvw[3],
                        const float pqr[3]) const
{
  for (int ii = 3; ii--;)
    pos[ii] = xyz[ii];
  for (int ii = 4; ii--;)
    attitude[ii] = quat[ii];

  // rotational rate and velocity are in the body axis system;
  // construct the rotation matrix
  float R_data[9];
  MatrixfE R(R_data, 3, 3);
  Q2R(R, quat);

  VectorfE v_velocity(velocity, 3);
  cVectorfE v_uvw(uvw, 3);
  VectorfE v_omega(omega, 3);
  cVectorfE v_pqr(pqr, 3);

  v_velocity = R * v_uvw;
  v_omega = R * v_pqr;
}

// new set-up
Orientation::Orientation(const EulerAngles &angles) :
  quat(&(this->L), 4)
{
  phithtpsi2Q(quat, angles.phi, angles.tht, angles.psi);
}

template <class V>
Orientation::Orientation(double angle, const V &axis) :
  L(cos(0.5 * angle)),
  lx(axis[0] * sin(0.5 * angle)),
  ly(axis[1] * sin(0.5 * angle)),
  lz(axis[2] * sin(0.5 * angle)),
  quat(&(this->L), 4)
{
  const double eps = 1.0e-8;
  double n2 = sqr(axis[0]) + sqr(axis[1]) + sqr(axis[2]);
  if (fabs(n2 - 1.0) > eps) {
    double n = 1.0 / sqrt(n2);
    lx *= n;
    ly *= n;
    lz *= n;
  }
}

Orientation::Orientation() :
  L(1.0),
  lx(0.0),
  ly(0.0),
  lz(0.0),
  quat(&(this->L), 4)
{
  //
}

Orientation::Orientation(const Orientation &o) :
  quat(&(this->L), 4)
{
#ifdef USING_EIGEN3
  this->quat = o.quat;
#else
  mtl::copy(o.quat, this->quat);
#endif
}

EulerAngles::EulerAngles(double phi, double tht, double psi) :
  phi(phi),
  tht(tht),
  psi(psi)
{
  //
}

EulerAngles::EulerAngles(const Orientation &o) :
  phi(Q2phi(o)),
  tht(Q2tht(o)),
  psi(Q2psi(o))
{
  //
}

Carthesian::Carthesian(double x, double y, double z) :
  x(x),
  y(y),
  z(z),
  xyz(&(this->x), 3)
{
  //
}

Carthesian::Carthesian(const Carthesian &o) :
  x(o.x),
  y(o.y),
  z(o.z),
  xyz(&(this->x), 3)
{
  //
}

Carthesian &Carthesian::operator=(const Carthesian &o)
{
  if (this != &o) {
    this->x = o.x;
    this->y = o.y;
    this->z = o.z;
    xyz = VectorE(&this->x, 3);
  }
  return *this;
}

ECEF::ECEF(double x, double y, double z) :
  Carthesian(x, y, z)
{
  assert(this->x == xyz[0]);
  assert(this->y == xyz[1]);
  assert(this->z == xyz[2]);
}

ECEF::ECEF(const LatLonAlt &lla) :
  Carthesian()
{
  double C = 1.0 / sqrt(sqr(cos(lla.lat)) + sqr(1 - f) * sqr(sin(lla.lat)));
  double S = sqr(1 - f) * C;
  x = (a * C + lla.alt) * cos(lla.lat) * cos(lla.lon);
  y = (a * C + lla.alt) * cos(lla.lat) * sin(lla.lon);
  z = (a * S + lla.alt) * sin(lla.lat);
}

ECEF::ECEF(const ECEF &o) :
  Carthesian(o.x, o.y, o.z)
{
  //
}

LatLonAlt::LatLonAlt(double lat, double lon, double alt) :
  lat(lat),
  lon(lon),
  alt(alt)
{
  //
}

LatLonAlt::LatLonAlt(const ECEF &ecef)
{
  // Bowring, B, 1976, Transformation from spatial to geographical coordinates
  // Survey Review, coordinates p 323-327
  // http://www.colorado.edu/geography/gcraft/notes/datum/gif/xyzllh.gif

  // implementation from:
  // http://www.mathworks.co.uk/matlabcentral/fileexchange/7941-convert-cartesian-ecef-coordinates-to-lat-lon-alt
  const double one_f = 1.0 - f;
  const double b = a * one_f;
  const double e2 = f * (2.0 - f);
  const double epsilon = e2 / (1.0 - e2);
  const double b_a = one_f;

  double p2 = sqr(ecef.x) + sqr(ecef.y);
  double r2 = p2 + sqr(ecef.z);
  double p = sqrt(p2);
  double r = sqrt(r2);

  double tanu = b_a * (ecef.z / p) * (1 + epsilon * b / r);
  double tan2u = sqr(tanu);

  double cos2u = 1.0 / (1.0 + tan2u);
  double cosu = sqrt(cos2u);
  double cos3u = cos2u * cosu;

  double sinu = tanu * cosu;
  double sin2u = 1.0 - cos2u;
  double sin3u = sin2u * sinu;

  double tanlat = (ecef.z + epsilon * b * sin3u) / (p - e2 * a * cos3u);

  double tan2lat = tanlat * tanlat;
  double cos2lat = 1.0 / (1.0 + tan2lat);
  double sin2lat = 1.0 - cos2lat;

  double coslat = sqrt(cos2lat);
  double sinlat = tanlat * coslat;

  lon = atan2(ecef.y, ecef.x);
  lat = atan(tanlat);
  alt = p * coslat + ecef.z * sinlat - a * sqrt(1 - e2 * sin2lat);
}

double LatLonAlt::RM() const
{
  const double esqr = f * (2.0 - f);
  return a * (1.0 + esqr * (1.5 * sin(lat) * sin(lat) - 1));
}

double LatLonAlt::RP() const
{
  const double esqr = f * (2.0 - f);
  return a * (1.0 + esqr * 0.5 * sin(lat) * sin(lat));
}

Orientation LatLonAlt::toGlobal(const double psi_zero) const
{
  // first system, ensure local axis points north
  double q1[] = { -cos(0.5 * psi_zero), 0.0, 0.0, sin(0.5 * psi_zero) };

  // 2nd system, from here the Z axis points North, x points "out"
  double q2[] = { -cos(0.5 * (lat + 0.5 * M_PI)), 0.0,
                  sin(0.5 * (lat + 0.5 * M_PI)), 0.0 };

  // 3rd system, now x is aligned with the gw meridian
  double q3[] = { -cos(0.5 * lon), 0.0, 0.0, sin(0.5 * lon) };

  double qtemp[4];
  QxQ(qtemp, q2, q1);
  Orientation result;
  QxQ(result, q3, qtemp);
  return result;
}

LocalAxis::LocalAxis(const LatLonAlt &lla, double psi_zero) :
  to_ECEF(3, 3),
  origin(lla),
  RM(lla.RM()),
  RP(lla.RP())
{
  Orientation qbase = lla.toGlobal(psi_zero);

  // set rotation matrix
  Q2R(to_ECEF, qbase);
}

ECEF LocalAxis::toECEF(const Carthesian &coords) const
{
  // correct for altitude error with parabolic approximation
  // (spherical earth), convert coordinates to ECEF and add the origin
  ECEF result(0, 0, 0);

  // horizontal distance to the local origin, squared
  double dsqr = sqr(coords.x) + sqr(coords.y);

  // first scale the x and y a little up with altitude (1+d/a), so
  // that higher flights far from the origin stay vertical above their
  // lat-lon ground intersection.
  Carthesian c2(coords);
  c2.x += c2.x / a * (-c2.z);
  c2.y += c2.y / a * (-c2.z);

  DEB(std::cout << "correction " << -0.5 / a * dsqr << std::endl);
  DEB(mult(to_ECEF, c2.xyz, result.xyz));
  DEB(add(result.xyz, origin.xyz, result.xyz));
  DEB(std::cout << "raw " << result << LatLonAlt(result) << std::endl);

  // following converts and adds the local vector to the origin
  // defined in ECEF already. An approximating correction is added for
  // distance from the local origin
#ifdef USING_EIGEN3
  result.xyz = to_ECEF * c2.xyz + (1.0 - 0.5 / sqr(a) * dsqr) * origin.xyz;
#else
  mult(to_ECEF, c2.xyz, scaled(origin.xyz, 1.0 - 0.5 / sqr(a) * dsqr),
       result.xyz);
#endif

#ifdef FINAL_ALTITUDE_CORRECTION
  // this is a further refinement; convert to lat, lon, alt, then correct
  // the z /alt coordinate, and return to ECEF, this corrects the final
  // z deviation from WGS84
  LatLonAlt lla(result);
  lla.alt = -coords.z;
  result = ECEF(lla);
#endif

  DEB(std::cout << "corr " << result << LatLonAlt(result) << std::endl);

  return result;
}

Carthesian LocalAxis::toLocal(const ECEF &ecef) const
{
  // subtract the origin. Correct altitude for distance, etc.
  Carthesian result;

#ifdef USING_EIGEN3
  result.xyz = to_ECEF.transpose() * (ecef.xyz - origin.xyz);
#else
#error "not corrected for mtl"
  add(scaled(origin.xyz, -1.0), ecef.xyz, result.xyz);
#endif

  // correction altitude
  double dsqr = sqr(result.x) + sqr(result.y);
  result.z -= 0.5 / a * dsqr;

  // correction x, y
  result.x -= result.x / a * (-result.z);
  result.y -= result.y / a * (-result.z);

  return result;
}

namespace std {

ostream &operator<<(ostream &os, const Carthesian &c)
{
  return os << "Carthesian(x=" << c.x << ", y=" << c.y << ", z=" << c.z << ")";
}

ostream &operator<<(ostream &os, const LatLonAlt &lla)
{
  return os << "LatLonAlt(lat=" << rad2deg(lla.lat)
            << ", lon=" << rad2deg(lla.lon) << ", alt=" << lla.alt << ")";
}

ostream &operator<<(ostream &os, const EulerAngles &ang)
{
  return os << "EulerAngles(phi=" << rad2deg(ang.phi)
            << ", tht=" << rad2deg(ang.tht) << ", psi=" << rad2deg(ang.psi)
            << ")";
}

ostream &operator<<(ostream &os, const Orientation &c)
{
  return os << "Orientation(L=" << c.L << ", lx=" << c.lx << ", ly=" << c.ly
            << ", lz=" << c.lz << ")";
}
} // namespace std

#ifdef DEBUG_AXIS
using namespace std;

static void print1(double result[6])
{
  cout << "lat=" << result[0] << " lon=" << result[1] << " alt=" << result[2]
       << " phi=" << result[3] << " tht=" << result[4] << " psi=" << result[5]
       << endl
       << endl;
}

static void print2(double pos[3], float vel[3], float att[4], float omg[3])
{
  cout << "x=" << pos[0] << " y=" << pos[1] << " z=" << pos[2];
  cout << " R=" << sqrt(sqr(pos[0]) + sqr(pos[1]) + sqr(pos[2])) << endl;
  cout << "u=" << vel[0] << " v=" << vel[1] << " w=" << vel[2];
  cout << " V=" << sqrt(sqr(vel[0]) + sqr(vel[1]) + sqr(vel[2])) << endl;
  cout << "phi=" << Q2phi(att);
  cout << " tht=" << Q2tht(att);
  cout << " psi=" << Q2psi(att) << endl;
  cout << "p=" << omg[0] << " q=" << omg[1] << " r=" << omg[2];
  cout << " O=" << sqrt(sqr(omg[0]) + sqr(omg[1]) + sqr(omg[2])) << endl
       << endl;
}

int main()
{
  {
    cout << "ECEF test 1" << endl;
    ECEF e0(a, 0, 0); // On equator, flying north, level
    Orientation quat(EulerAngles(0.0, deg2rad(-90.0), 0.0));
    double uvw[] = { 8.0, 0.0, 0.0 }; // in body
    double pqr[] = { 1.0, 0.0, 0.0 }; // in body
    double pos[3], result[6];
    float vel[3];
    float att[4];
    float omg[3];
    FGECEFAxis a;
    a.transform(result, e0.xyz.data(), quat.quat.data());
    a.toECEF(pos, vel, att, omg, e0.xyz.data(), quat.quat.data(), uvw, pqr);
    print1(result);
    print2(pos, vel, att, omg);
  }

  {
    cout << "ECEF test 2" << endl;
    ECEF e0(LatLonAlt(deg2rad(30.0), deg2rad(0.0), 1000.0));
    Orientation quat(EulerAngles(0.0, deg2rad(-100.0), 0.0));
    double uvw[] = { 8.0, 0.0, 2.0 }; // in body
    double pqr[] = { 0.0, 1.0, 0.0 }; // in body
    double pos[3], result[6];
    float vel[3];
    float att[4];
    float omg[3];
    FGECEFAxis a;
    a.transform(result, e0.xyz.data(), quat.quat.data());
    a.toECEF(pos, vel, att, omg, e0.xyz.data(), quat.quat.data(), uvw, pqr);
    print1(result);
    print2(pos, vel, att, omg);
  }

#if 0
  // point tests
  {
    LatLonAlt lla(0, deg2rad(90.0), 0);
    ECEF e2(lla);
    cout << e2 << endl;
    LatLonAlt l2(e2);
    cout << lla << ' ' << l2 << endl << e2 << endl;
  }

  {
    LatLonAlt lla(deg2rad(30.0), 0, 0);
    ECEF e2(lla);
    LatLonAlt l2(e2);
    cout << lla << ' ' << l2 << endl << e2 << endl;
  }



  {
    Carthesian c(60*1852, 0, 0);
    cout << "Local:" << c << endl;

    LocalAxis ax(LatLonAlt(0, 0, 0), 0);

    ECEF ce = ax.toECEF(c);
    LatLonAlt cl(ce);
    cout << "ECEF :" << ce << endl;
    cout << "LLA  :" << cl << endl;
  }

  {
    Carthesian c(0, 10*1852, 0);
    cout << "Local:" << c << endl;

    LocalAxis ax(LatLonAlt(deg2rad(35.0), 0, 0), 0);

    ECEF ce = ax.toECEF(c);
    LatLonAlt cl(ce);
    cout << "ECEF :" << ce << endl;
    cout << "LLA  :" << cl << endl;
  }



  {
    // local axis on the equator, +10 deg, i.e. east
    FGLocalAxis a(0.0, 0.0, deg2rad(0.0));

    // test 1, x 60 miles, y 0 m
    double xyz1[] = { 60*1852, 0, 0};
    double quat1[4]; phithtpsi2Q(quat1, 0.0, 0.0, 0.0);

    double result[6];
    a.transform(result, xyz1, quat1);
    print1(result);

    // test 1b, with ECEF conversion
    double uvw[3] = { 10.0, 0.0, 0.0};
    double pqr[3] = { 0.1, 0.0, 0.0};

    double pos[3]; float vel[3]; float att[4]; float omg[3];
    a.toECEF(pos, vel, att, omg, xyz1, quat1, uvw, pqr);
    print2(pos, vel, att, omg);

    // test 2, y 60 miles
    double xyz2[] = { 0, 60*1852, 0};
    a.transform(result, xyz2, quat1);
    print1(result);

    // test 3, 1000 m high wrt previous
    xyz2[2] = -1000.0;
    a.transform(result, xyz2, quat1);
    print1(result);
  }

  {
    // local axis 30 deg up, -10 deg, i.e. west
    FGLocalAxis a(deg2rad(30.0), 0.0, deg2rad(-10.0));

    // test 1, x 60 miles, y 0 m
    double xyz1[] = { 60*1852, 0, 0};
    double quat1[4]; phithtpsi2Q(quat1, 0.0, 0.0, 0.0);

    double result[6];
    a.transform(result, xyz1, quat1);
    print1(result);

    // test 2, y 60 miles
    double xyz2[] = { 0, 60*1852, 0};
    a.transform(result, xyz2, quat1);
    print1(result);

    // test 3, 1000 m high wrt previous
    xyz2[2] = -1000.0;
    a.transform(result, xyz2, quat1);
    print1(result);
  }

  {
    // local axis 30 deg up, -10 deg, i.e. west
    FGLocalAxis a(deg2rad(-30.0), deg2rad(45.0), deg2rad(90.0));

    // test 1, x 60 miles, y 0 m
    double xyz1[] = { 60*1852, 0, 0};
    double quat1[4]; phithtpsi2Q(quat1, 0.0, deg2rad(5.0), 0.0);

    double result[6];
    a.transform(result, xyz1, quat1);
    print1(result);

    // test 2, y 60 miles
    double xyz2[] = { 0, 60*1852, 0};
    a.transform(result, xyz2, quat1);
    print1(result);

    // test 3, 1000 m high wrt previous
    xyz2[2] = -1000.0;
    a.transform(result, xyz2, quat1);
    print1(result);
  }
#endif
}

#endif

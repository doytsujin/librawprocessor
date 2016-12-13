#ifndef __RAWSCALE_H__
#define __RAWSCALE_H__

#include <cstddef>
#include <cmath>
#include <cstring>

////////////////////////////////////////////////////////////////////////////////
//
// RAWSCALE
// ============================================================================
// Reprogrammed by Raphael Kim (rageworx@gmail.com) for librawprocessor.
//
// * This class was belongs to below project:
//
//    ==========================================================
//    FreeImage 3
//    ----------------------------------------------------------
//    Design and implementation by
//    - Floris van den Berg (flvdberg@wxs.nl)
//    - Herv?Drolon (drolon@infonie.fr)
//
//    ==========================================================
//
// * History of updates *
//
// [2016-11-11]
//   - Modified for librawprocessor.
//
////////////////////////////////////////////////////////////////////////////////

// Filters
class RAWGenericFilter
{
    protected:
        #define FILTER_PI  double (3.1415926535897932384626433832795)
        #define FILTER_2PI double (2.0 * 3.1415926535897932384626433832795)
        #define FILTER_4PI double (4.0 * 3.1415926535897932384626433832795)

    protected:
        double  _dWidth;

    public:
        RAWGenericFilter (double dWidth) : _dWidth (dWidth) {}
        virtual ~RAWGenericFilter() {}

        double GetWidth()                   { return _dWidth; }
        void   SetWidth (double dWidth)     { _dWidth = dWidth; }
        virtual double Filter (double dVal) = 0;
};

class RAWBoxFilter : public RAWGenericFilter
{
    public:
        // Default fixed width = 0.5
        RAWBoxFilter() : RAWGenericFilter(0.5) {}
        virtual ~RAWBoxFilter() {}

    public:
        double Filter (double dVal)
        { return ( fabs(dVal) <= _dWidth ? 1.0 : 0.0 ); }
};

class RAWBilinearFilter : public RAWGenericFilter
{
    public:
        RAWBilinearFilter () : RAWGenericFilter(1) {}
        virtual ~RAWBilinearFilter() {}

    public:
        double Filter (double dVal)
        {
            dVal = fabs( dVal );
            return ( dVal < _dWidth ? _dWidth - dVal : 0.0 );
        }
};

class RAWBicubicFilter : public RAWGenericFilter
{
    protected:
        // data for parameterized Mitchell filter
        double p0, p2, p3;
        double q0, q1, q2, q3;

    public:
        // Default fixed width = 2
        RAWBicubicFilter ( double b = ( 1 / (double)3 ), double c = ( 1 / (double)3 ) )
         : RAWGenericFilter(2)
        {
            p0 = (   6 - 2 * b ) / 6;
            p2 = ( -18 + 12 * b + 6 * c ) / 6;
            p3 = (  12 - 9 * b - 6 * c ) / 6;
            q0 = (   8 * b + 24 * c ) / 6;
            q1 = ( -12 * b - 48 * c ) / 6;
            q2 = (   6 * b + 30 * c ) / 6;
            q3 = (  -b - 6 * c ) / 6;
        }
        virtual ~RAWBicubicFilter() {}

    public:
        double Filter(double dVal)
        {
            dVal = fabs(dVal);

            if(dVal < 1)
                return ( p0 + dVal * dVal * ( p2 + dVal * p3 ) );

            if(dVal < 2)
                return ( q0 + dVal * ( q1 + dVal * ( q2 + dVal * q3 ) ) );

            return 0;
        }
};

class RAWCatmullRomFilter : public RAWGenericFilter
{
    public:
        // Default fixed width = 2
        RAWCatmullRomFilter() : RAWGenericFilter(2) {}
        virtual ~RAWCatmullRomFilter() {}

    public:
        double Filter(double dVal)
        {
            if( dVal < -2 )
                return 0;

            if( dVal < -1 )
                return ( 0.5 * ( 4 + dVal * ( 8 + dVal * ( 5 + dVal ) ) ) );

            if( dVal < 0  )
                return ( 0.5 * ( 2 + dVal * dVal * ( -5 - 3 * dVal ) ) );

            if( dVal < 1  )
                return ( 0.5 * ( 2 + dVal * dVal * ( -5 + 3 * dVal ) ) );

            if( dVal < 2  )
                return ( 0.5 * ( 4 + dVal * ( -8 + dVal * ( 5 - dVal ) ) ) );

            return 0;
        }
};

class RAWLanczos3Filter : public RAWGenericFilter
{
    public:
        // Default fixed width = 3
        RAWLanczos3Filter() : RAWGenericFilter(3) {}
        virtual ~RAWLanczos3Filter() {}

    public:
        double Filter(double dVal)
        {
            dVal = fabs(dVal);
            if( dVal < _dWidth )
            {
                return ( sinc( dVal ) * sinc( dVal / _dWidth ) );
            }
            return 0;
        }

    private:
        double sinc( double value )
        {
            if( value != 0 )
            {
                value *= FILTER_PI;
                return (sin(value) / value);
            }
            return 1;
        }
};

class RAWBSplineFilter : public RAWGenericFilter
{
    public:
        // Default fixed width = 2
        RAWBSplineFilter() : RAWGenericFilter(2) {}
        virtual ~RAWBSplineFilter() {}

    public:
        double Filter( double dVal )
        {
            dVal = fabs(dVal);
            if( dVal < 1 ) return ( 4 + dVal*dVal*(-6 + 3 * dVal ) ) / 6;
            if( dVal < 2 )
            {
                double t = 2 - dVal;
                return ( t * t * t / 6);
            }
            return 0;
        }
};

class RAWBlackmanFilter : public RAWGenericFilter
{
    public:
        /**
        Constructor<br>
        Default width = 0.5
        */
        RAWBlackmanFilter( double dWidth = double(0.5) )
         : RAWGenericFilter(dWidth) {}
        virtual ~RAWBlackmanFilter() {}

    public:
        double Filter (double dVal)
        {
            if(fabs (dVal) > _dWidth)
                return 0;

            double dN = 2 * _dWidth + 1;
            dVal /= ( dN - 1 );

            return 0.42 + 0.5 * cos( FILTER_2PI * dVal ) +
                   0.08 * cos( FILTER_4PI * dVal );
        }
};

////////////////////////////////////////////////////////////////////////////////
// Resize relations.

class RawScaleWeightsTable
{
    typedef struct
    {
        double*     Weights;
        unsigned    Left;
        unsigned    Right;
    }Contribution;

    private:
        Contribution*   _WeightTable;
        unsigned        _WindowSize;
        unsigned        _LineLength;

    public:
        RawScaleWeightsTable( RAWGenericFilter* pFilter = NULL, unsigned uDstSize = 0, unsigned uSrcSize = 0 );
        ~RawScaleWeightsTable();

    public:
        double   getWeight( unsigned dst_pos, unsigned src_pos );
        unsigned getLeftBoundary( unsigned dst_pos );
        unsigned getRightBoundary( unsigned dst_pos );
};

class RAWResizeEngine
{
    private:
        RAWGenericFilter* _pFilter;

    public:
        RAWResizeEngine( RAWGenericFilter* filter = NULL );
        virtual ~RAWResizeEngine() {}

    public:
        unsigned scale( const unsigned short* src, unsigned src_width, unsigned src_height,
                        unsigned dst_width, unsigned dst_height, unsigned short** dst );

    private:
        void horizontalFilter( const unsigned short* src, const unsigned height, const unsigned src_width,
                               const unsigned src_offset_x, const unsigned src_offset_y,
                               unsigned short* dst, const unsigned dst_width);
        void verticalFilter( const unsigned short* src, const unsigned width, const unsigned src_height,
                             const unsigned src_offset_x, const unsigned src_offset_y,
                             unsigned short* dst, const unsigned dst_width, const unsigned dst_height);
};


#endif /// of __RAWSCALE_H__
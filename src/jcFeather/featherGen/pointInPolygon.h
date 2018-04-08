#ifndef _pointInPolygon_
#define _pointInPolygon_

#include	<math.h>

namespace jerryC{

	#define DIM     2               /* Dimension of points */
	typedef int     tPointi[DIM];   /* type integer point */
	typedef double  tPointd[DIM];   /* type double point */
	#define PMAX    30           /* Max # of pts in polygon */
	   
	typedef tPointd tPolygoni[PMAX];/* type integer polygon */

	inline char InPoly( tPointd q, tPolygoni P, int n )
	{
	  int	 i, i1;      /* point index; i1 = i-1 mod n */
	  int	 d;          /* dimension index */
	  double x;          /* x intersection of e with ray */
	  int	 Rcross = 0; /* number of right edge/ray crossings */
	  int    Lcross = 0; /* number of left edge/ray crossings */

	  
	  /* Shift so that q is the origin. Note this destroys the polygon.
		 This is done for pedogical clarity. */
	  for( i = 0; i < n; i++ ) {
		for( d = 0; d < DIM; d++ )
		  P[i][d] = P[i][d] - q[d];
	  }
		
	  /* For each edge e=(i-1,i), see if crosses ray. */
	  for( i = 0; i < n; i++ ) {
		/* First see if q=(0,0) is a vertex. */
		if ( P[i][0]==0 && P[i][1]==0 ) return 'v';
		i1 = ( i + n - 1 ) % n;	   
	    
		if( ( P[i][1] > 0 ) != ( P[i1][1] > 0 ) ) {
	      
		  /* e straddles ray, so compute intersection with ray. */
		  x = (P[i][0] * (double)P[i1][1] - P[i1][0] * (double)P[i][1])
		/ (double)(P[i1][1] - P[i][1]);
		  /* printf("straddles: x = %g\t", x); */
	      
		  /* crosses ray if strictly positive intersection. */
		  if (x > 0) Rcross++;
		}
	    
		if ( ( P[i][1] < 0 ) != ( P[i1][1] < 0 ) ) { 
	      
		  /* e straddles ray, so compute intersection with ray. */
		  x = (P[i][0] * P[i1][1] - P[i1][0] * P[i][1])
			  / (double)(P[i1][1] - P[i][1]);

		  /* crosses ray if strictly positive intersection. */
		  if (x < 0) Lcross++;
		}
		/* printf("Left cross=%d\n", Lcross); */
	  }	
	  
	  /* q on the edge if left and right cross are not the same parity. */
	  if( ( Rcross % 2 ) != (Lcross % 2 ) )
		return 'e';
	  
	  /* q inside iff an odd number of crossings. */
	  if( (Rcross % 2) == 1 )
		return 'i';
	  else	return 'o';
	}


};

#endif
//
//  vas_window_hann.h
//  vas_binauralrir_unity
//
//  Created by Hannes on 19.01.21.
//

/*
 simple js script to get a hann window:
 n = 64;
 half = true //only output first half of window. useful if window is symmetric (save 50% memory)
 win = [];
 win[0] = 0;
 win[n-1] = 0;
 output = "";
 for(var i = 1; i < n/2; i++) {
     val = Math.pow( Math.sin( (Math.PI*i) / n ), 2 );
     win[i] = val;
     win[n - 1 - i] = val;
 }
 c = half ? n / 2 : n;
 output += "{";
 for(var i = 0; i < c - 1; i++) {
    output += ( win[i] + "f, " );
 }
 output += ( win[c-1] + "}" );
 console.log(output);
 */

#ifndef vas_window_hann_h
#define vas_window_hann_h

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Returns a single-precision array containing a Hanning window of the desired size. Lookup tables are used internally.
 *
 * @param n The desired size of the window.
 *
 * @return A single-precision array of size (n/2) containing a Hanning window of the desired size. Since The window is symmetrical, only the first half of the points are provided.
 *
 */
const float *vas_window_hann(int n);

#ifdef __cplusplus
}
#endif

#endif /* vas_window_hann_h */

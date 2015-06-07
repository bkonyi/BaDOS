#ifndef __BWIO_H__
#define __BWIO_H__
/*
 * bwio.h
 */

int bwsetfifo( int channel, int state );

int bwsetspeed( int channel, int speed );

int bwputc( int channel, char c );

int bwgetc( int channel );

int bwputx( int channel, char c );

int bwputstr( int channel, char *str );

int bwputr( int channel, unsigned int reg );

void bwputw( int channel, int n, char fc, char *bf );

void bwprintf( int channel, char *format, ... );

//Our functions

void bwdumpregs();

#endif //__BWIO_H__

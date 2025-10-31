// 
// Stingray Software Extension Classes
// Copyright (C) 1995 Stingray Software Inc.
// All Rights Reserved
// 
// This source code is only intended as a supplement to the
// Stingray Extension Class product.
// See the SEC help files for detailed information
// regarding using SEC classes.
// 
//  Author:		AAB 
//		Originally based on implementation by Sam Leffler
//		Copyright (c) 1988, 1989, 1990, 1991, 1992 Sam Leffler
//		Copyright (c) 1991, 1992 Silicon Graphics, Inc.
//		
//  Description:	tif_prin.cpp
//		directory printing routines
//
//  Created:		Tue Aug 29 15:15:44 1995
//
//  CHANGELOG:
//
// 	AAB 7/1/95	Initial implementation
//
//


#include "..\..\..\stdafx.h"

#ifndef __TIFFIOP_H__
#include "..\..\..\Include\Image\tiffiop.h"
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef __SECTIFF_H__
#include "..\..\..\Include\Image\sectiff.h"
#endif


#ifdef JPEG_SUPPORT
static void
JPEGPrintQTable(FILE* fd, LPBYTE tab)
{
	int i, j;
	TCHAR *sep;

	_fputtc('\n', fd);
	for (i = 0; i < 8; i++) {
		sep = _T("    ");
		for (j = 0; j < 8; j++) {
			_ftprintf(fd, _T("%s%2u"), sep, tab[8*i+j]);
			sep = _T(", ");
		}
		_fputtc('\n', fd);
	}
}

static void
JPEGPrintCTable(FILE* fd, LPBYTE tab)
{
	int i, n, count;
	TCHAR *sep=NULL;

	_ftprintf(fd, _T("\n    Bits:"));
	count = 0;
	for (i = 0; i < 16; i++) {
		_ftprintf(fd, _T(" %u"), tab[i]);
		count += tab[i];
	}
	n = 0;
	for (; count > 0; count--) {
		if ((n % 8) == 0) {
			_fputtc('\n', fd);
			sep = _T("    ");
		}
		_ftprintf(fd, _T("%s0x%02x"), sep, tab[i++]);
		sep = _T(", ");
		n++;

	}
	if (n % 8)
		_fputtc('\n', fd);
}
#endif

static const TCHAR *photoNames[] = {
    _T("min-is-white"),				/* PHOTOMETRIC_MINISWHITE */
    _T("min-is-black"),				/* PHOTOMETRIC_MINISBLACK */
    _T("RGB color"),				/* PHOTOMETRIC_RGB */
    _T("palette color (RGB from colormap)"),	/* PHOTOMETRIC_PALETTE */
    _T("transparency mask"),			/* PHOTOMETRIC_MASK */
    _T("separated"),				/* PHOTOMETRIC_SEPARATED */
    _T("YCbCr"),					/* PHOTOMETRIC_YCBCR */
    _T("7 (0x7)"),
    _T("CIE L*a*b*"),				/* PHOTOMETRIC_CIELAB */
};
#define	NPHOTONAMES	(sizeof (photoNames) / sizeof (photoNames[0]))

static const TCHAR *orientNames[] = {
    _T("0 (0x0)"),
    _T("row 0 top, col 0 lhs"),			/* ORIENTATION_TOPLEFT */
    _T("row 0 top, col 0 rhs"),			/* ORIENTATION_TOPRIGHT */
    _T("row 0 bottom, col 0 rhs"),			/* ORIENTATION_BOTRIGHT */
    _T("row 0 bottom, col 0 lhs"),			/* ORIENTATION_BOTLEFT */
    _T("row 0 lhs, col 0 top"),			/* ORIENTATION_LEFTTOP */
    _T("row 0 rhs, col 0 top"),			/* ORIENTATION_RIGHTTOP */
    _T("row 0 rhs, col 0 bottom"),			/* ORIENTATION_RIGHTBOT */
    _T("row 0 lhs, col 0 bottom"),			/* ORIENTATION_LEFTBOT */
};
#define	NORIENTNAMES	(sizeof (orientNames) / sizeof (orientNames[0]))

/*
 * Print the contents of the current directory
 * to the specified stdio file stream.
 */
void
SECTiff::TIFFPrintDirectory(TIFF* tif, FILE* fd, long flags)
{
	register TIFFDirectory *td;
	TCHAR *sep;
	WORD i, j;
	WORD n;

	_ftprintf(fd, _T("TIFF Directory at offset 0x%x\n"), tif->tif_diroff);
	td = &tif->tif_dir;
	if (TIFFFieldSet(tif,FIELD_SUBFILETYPE)) {
		_ftprintf(fd, _T("  Subfile Type:"));
		sep = _T(" ");
		if (td->td_subfiletype & FILETYPE_REDUCEDIMAGE) {
			_ftprintf(fd, _T("%sreduced-resolution image"), sep);
			sep = _T("/");
		}
		if (td->td_subfiletype & FILETYPE_PAGE) {
			_ftprintf(fd, _T("%smulti-page document"), sep);
			sep = _T("/");
		}
		if (td->td_subfiletype & FILETYPE_MASK)
			_ftprintf(fd, _T("%stransparency mask"), sep);
		_ftprintf(fd, _T(" (%u = 0x%x)\n"),
		    td->td_subfiletype, td->td_subfiletype);
	}
	if (TIFFFieldSet(tif,FIELD_IMAGEDIMENSIONS)) {
		_ftprintf(fd, _T("  Image Width: %lu Image Length: %lu"),
		    (DWORD) td->td_imagewidth, (DWORD) td->td_imagelength);
		if (TIFFFieldSet(tif,FIELD_IMAGEDEPTH))
			_ftprintf(fd, _T(" Image Depth: %lu"),
			    (DWORD) td->td_imagedepth);
		_ftprintf(fd, _T("\n"));
	}
	if (TIFFFieldSet(tif,FIELD_TILEDIMENSIONS)) {
		_ftprintf(fd, _T("  Tile Width: %lu Tile Length: %lu"),
		    (DWORD) td->td_tilewidth, (DWORD) td->td_tilelength);
		if (TIFFFieldSet(tif,FIELD_TILEDEPTH))
			_ftprintf(fd, _T(" Tile Depth: %lu"),
			    (DWORD) td->td_tiledepth);
		_ftprintf(fd, _T("\n"));
	}
	if (TIFFFieldSet(tif,FIELD_RESOLUTION)) {
		_ftprintf(fd, _T("  Resolution: %g, %g"),
		    td->td_xresolution, td->td_yresolution);
		if (TIFFFieldSet(tif,FIELD_RESOLUTIONUNIT)) {
			switch (td->td_resolutionunit) {
			case RESUNIT_NONE:
				_ftprintf(fd, _T(" (unitless)"));
				break;
			case RESUNIT_INCH:
				_ftprintf(fd, _T(" pixels/inch"));
				break;
			case RESUNIT_CENTIMETER:
				_ftprintf(fd, _T(" pixels/cm"));
				break;
			default:
				_ftprintf(fd, _T(" (unit %u = 0x%x)"),
				    td->td_resolutionunit,
				    td->td_resolutionunit);
				break;
			}
		}
		_ftprintf(fd, _T("\n"));
	}
	if (TIFFFieldSet(tif,FIELD_POSITION))
		_ftprintf(fd, _T("  Position: %g, %g\n"),
		    td->td_xposition, td->td_yposition);
	if (TIFFFieldSet(tif,FIELD_BITSPERSAMPLE))
		_ftprintf(fd, _T("  Bits/Sample: %u\n"), td->td_bitspersample);
	if (TIFFFieldSet(tif,FIELD_SAMPLEFORMAT)) {
		_ftprintf(fd, _T("  Sample Format: "));
		switch (td->td_sampleformat) {
		case SAMPLEFORMAT_VOID:
			_ftprintf(fd, _T("void\n"));
			break;
		case SAMPLEFORMAT_INT:
			_ftprintf(fd, _T("signed integer\n"));
			break;
		case SAMPLEFORMAT_UINT:
			_ftprintf(fd, _T("unsigned integer\n"));
			break;
		case SAMPLEFORMAT_IEEEFP:
			_ftprintf(fd, _T("IEEE floating point\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_sampleformat, td->td_sampleformat);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_COMPRESSION)) {
		_ftprintf(fd, _T("  Compression Scheme: "));
		switch (td->td_compression) {
		case COMPRESSION_NONE:
			_ftprintf(fd, _T("none\n"));
			break;
		case COMPRESSION_CCITTRLE:
			_ftprintf(fd, _T("CCITT modified Huffman encoding\n"));
			break;
		case COMPRESSION_CCITTFAX3:
			_ftprintf(fd, _T("CCITT Group 3 facsimile encoding\n"));
			break;
		case COMPRESSION_CCITTFAX4:
			_ftprintf(fd, _T("CCITT Group 4 facsimile encoding\n"));
			break;
		case COMPRESSION_CCITTRLEW:
			_ftprintf(fd, _T("CCITT modified Huffman encoding %s\n"),
			    _T("w/ word alignment"));
			break;
		case COMPRESSION_PACKBITS:
			_ftprintf(fd, _T("Macintosh PackBits encoding\n"));
			break;
		case COMPRESSION_THUNDERSCAN:
			_ftprintf(fd, _T("ThunderScan 4-bit encoding\n"));
			break;
		case COMPRESSION_LZW:
			_ftprintf(fd, _T("Lempel-Ziv & Welch encoding\n"));
			break;
		case COMPRESSION_NEXT:
			_ftprintf(fd, _T("NeXT 2-bit encoding\n"));
			break;
		case COMPRESSION_JPEG:
			_ftprintf(fd, _T("JPEG encoding\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_compression, td->td_compression);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_PHOTOMETRIC)) {
		_ftprintf(fd, _T("  Photometric Interpretation: "));
		if (td->td_photometric < NPHOTONAMES)
			_ftprintf(fd, _T("%s\n"), photoNames[td->td_photometric]);
		else
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_photometric, td->td_photometric);
	}
	if (TIFFFieldSet(tif,FIELD_EXTRASAMPLES) && td->td_extrasamples) {
		_ftprintf(fd, _T("  Extra Samples: %u<"), td->td_extrasamples);
		sep = _T("");
		for (i = 0; i < td->td_extrasamples; i++) {
			switch (td->td_sampleinfo[i]) {
			case EXTRASAMPLE_UNSPECIFIED:
				_ftprintf(fd, _T("%sunspecified"), sep);
				break;
			case EXTRASAMPLE_ASSOCALPHA:
				_ftprintf(fd, _T("%sassoc-alpha"), sep);
				break;
			case EXTRASAMPLE_UNASSALPHA:
				_ftprintf(fd, _T("%sunassoc-alpha"), sep);
				break;
			default:
				_ftprintf(fd, _T("%s%u (0x%x)"), sep,
				    td->td_sampleinfo[i], td->td_sampleinfo[i]);
				break;
			}
			sep = _T(", ");
		}
		_ftprintf(fd, _T(">\n"));
	}
#ifdef CMYK_SUPPORT
	if (TIFFFieldSet(tif,FIELD_INKSET)) {
		_ftprintf(fd, _T("  Ink Set: "));
		switch (td->td_inkset) {
		case INKSET_CMYK:
			_ftprintf(fd, _T("CMYK\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_inkset, td->td_inkset);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_INKNAMES)) {
		TCHAR *cp;
		_ftprintf(fd, _T("  Ink Names: "));
		i = td->td_samplesperpixel;
		sep = _T("");
		for (cp = td->td_inknames; i > 0; cp = _tcschr(cp, '\0')) {
			_ftprintf(fd, _T("%s%s"), sep, cp);
			sep = _T(", ");
		}
	}
	if (TIFFFieldSet(tif,FIELD_DOTRANGE))
		_ftprintf(fd, _T("  Dot Range: %u-%u\n"),
		    td->td_dotrange[0], td->td_dotrange[1]);
	if (TIFFFieldSet(tif,FIELD_TARGETPRINTER))
		_ftprintf(fd, _T("  Target Printer: %s\n"), td->td_targetprinter);
#endif
	if (TIFFFieldSet(tif,FIELD_THRESHHOLDING)) {
		_ftprintf(fd, _T("  Thresholding: "));
		switch (td->td_threshholding) {
		case THRESHHOLD_BILEVEL:
			_ftprintf(fd, _T("bilevel art scan\n"));
			break;
		case THRESHHOLD_HALFTONE:
			_ftprintf(fd, _T("halftone or dithered scan\n"));
			break;
		case THRESHHOLD_ERRORDIFFUSE:
			_ftprintf(fd, _T("error diffused\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_threshholding, td->td_threshholding);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_FILLORDER)) {
		_ftprintf(fd, _T("  FillOrder: "));
		switch (td->td_fillorder) {
		case FILLORDER_MSB2LSB:
			_ftprintf(fd, _T("msb-to-lsb\n"));
			break;
		case FILLORDER_LSB2MSB:
			_ftprintf(fd, _T("lsb-to-msb\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_fillorder, td->td_fillorder);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_PREDICTOR)) {
		_ftprintf(fd, _T("  Predictor: "));
		switch (td->td_predictor) {
		case 1:
			_ftprintf(fd, _T("none\n"));
			break;
		case 2:
			_ftprintf(fd, _T("horizontal differencing\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_predictor, td->td_predictor);
			break;
		}
	}
#ifdef YCBCR_SUPPORT
	if (TIFFFieldSet(tif,FIELD_YCBCRSUBSAMPLING))
		_ftprintf(fd, _T("  YCbCr Subsampling: %u, %u\n"),
		    td->td_ycbcrsubsampling[0], td->td_ycbcrsubsampling[1]);
	if (TIFFFieldSet(tif,FIELD_YCBCRPOSITIONING)) {
		_ftprintf(fd, _T("  YCbCr Positioning: "));
		switch (td->td_ycbcrpositioning) {
		case YCBCRPOSITION_CENTERED:
			_ftprintf(fd, _T("centered\n"));
			break;
		case YCBCRPOSITION_COSITED:
			_ftprintf(fd, _T("cosited\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_ycbcrpositioning, td->td_ycbcrpositioning);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_YCBCRCOEFFICIENTS))
		_ftprintf(fd, _T("  YCbCr Coefficients: %g, %g, %g\n"),
		    td->td_ycbcrcoeffs[0],
		    td->td_ycbcrcoeffs[1],
		    td->td_ycbcrcoeffs[2]);
#endif
#ifdef JPEG_SUPPORT
	if (TIFFFieldSet(tif,FIELD_JPEGPROC)) {
		_ftprintf(fd, _T("  JPEG Processing Mode: "));
		switch (td->td_jpegproc) {
		case JPEGPROC_BASELINE:
			_ftprintf(fd, _T("baseline sequential algorithm\n"));
			break;
		case JPEGPROC_LOSSLESS:
			_ftprintf(fd, _T("lossless algorithm with Huffman coding\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_jpegproc, td->td_jpegproc);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_JPEGRESTARTINTERVAL)) {
		_ftprintf(fd, _T("  JPEG Restart Interval: "));
		if (td->td_jpegrestartinterval)
			_ftprintf(fd, _T("%u\n"), td->td_jpegrestartinterval);
		else
			_ftprintf(fd, _T("(no restart markers)\n"));
	}
	if (TIFFFieldSet(tif,FIELD_JPEGQTABLES)) {
		_ftprintf(fd, _T("  JPEG Quantization Tables: "));
		if (flags & TIFFPRINT_JPEGQTABLES) {
			for (i = 0; i < td->td_samplesperpixel; i++)
				JPEGPrintQTable(fd, td->td_qtab[i]);
		} else
			_ftprintf(fd, _T("(present)\n"));
	}
	if (TIFFFieldSet(tif,FIELD_JPEGDCTABLES)) {
		_ftprintf(fd, _T("  JPEG DC Tables: "));
		if (flags & TIFFPRINT_JPEGDCTABLES) {
			for (i = 0; i < td->td_samplesperpixel; i++)
				JPEGPrintCTable(fd, td->td_dctab[i]);
		} else
			_ftprintf(fd, _T("(present)\n"));
	}
	if (TIFFFieldSet(tif,FIELD_JPEGACTABLES)) {
		_ftprintf(fd, _T("  JPEG AC Tables: "));
		if (flags & TIFFPRINT_JPEGACTABLES) {
			for (i = 0; i < td->td_samplesperpixel; i++)
				JPEGPrintCTable(fd, td->td_actab[i]);
		} else
			_ftprintf(fd, _T("(present)\n"));
	}
#endif
	if (TIFFFieldSet(tif,FIELD_HALFTONEHINTS))
		_ftprintf(fd, _T("  Halftone Hints: light %u dark %u\n"),
		    td->td_halftonehints[0], td->td_halftonehints[1]);
	if (TIFFFieldSet(tif,FIELD_ARTIST))
		_ftprintf(fd, _T("  Artist: \"%s\"\n"), td->td_artist);
	if (TIFFFieldSet(tif,FIELD_DATETIME))
		_ftprintf(fd, _T("  Date & Time: \"%s\"\n"), td->td_datetime);
	if (TIFFFieldSet(tif,FIELD_HOSTCOMPUTER))
		_ftprintf(fd, _T("  Host Computer: \"%s\"\n"), td->td_hostcomputer);
	if (TIFFFieldSet(tif,FIELD_SOFTWARE))
		_ftprintf(fd, _T("  Software: \"%s\"\n"), td->td_software);
	if (TIFFFieldSet(tif,FIELD_DOCUMENTNAME))
		_ftprintf(fd, _T("  Document Name: \"%s\"\n"), td->td_documentname);
	if (TIFFFieldSet(tif,FIELD_IMAGEDESCRIPTION))
		_ftprintf(fd, _T("  Image Description: \"%s\"\n"),
		    td->td_imagedescription);
	if (TIFFFieldSet(tif,FIELD_MAKE))
		_ftprintf(fd, _T("  Make: \"%s\"\n"), td->td_make);
	if (TIFFFieldSet(tif,FIELD_MODEL))
		_ftprintf(fd, _T("  Model: \"%s\"\n"), td->td_model);
	if (TIFFFieldSet(tif,FIELD_ORIENTATION)) {
		_ftprintf(fd, _T("  Orientation: "));
		if (td->td_orientation < NORIENTNAMES)
			_ftprintf(fd, _T("%s\n"), orientNames[td->td_orientation]);
		else
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_orientation, td->td_orientation);
	}
	if (TIFFFieldSet(tif,FIELD_SAMPLESPERPIXEL))
		_ftprintf(fd, _T("  Samples/Pixel: %u\n"), td->td_samplesperpixel);
	if (TIFFFieldSet(tif,FIELD_ROWSPERSTRIP)) {
		_ftprintf(fd, _T("  Rows/Strip: "));
		if (td->td_rowsperstrip == 0xffffffffL)
			_ftprintf(fd, _T("(infinite)\n"));
		else
			_ftprintf(fd, _T("%lu\n"), (DWORD) td->td_rowsperstrip);
	}
	if (TIFFFieldSet(tif,FIELD_MINSAMPLEVALUE))
		_ftprintf(fd, _T("  Min Sample Value: %u\n"), td->td_minsamplevalue);
	if (TIFFFieldSet(tif,FIELD_MAXSAMPLEVALUE))
		_ftprintf(fd, _T("  Max Sample Value: %u\n"), td->td_maxsamplevalue);
	if (TIFFFieldSet(tif,FIELD_PLANARCONFIG)) {
		_ftprintf(fd, _T("  Planar Configuration: "));
		switch (td->td_planarconfig) {
		case PLANARCONFIG_CONTIG:
			_ftprintf(fd, _T("single image plane\n"));
			break;
		case PLANARCONFIG_SEPARATE:
			_ftprintf(fd, _T("separate image planes\n"));
			break;
		default:
			_ftprintf(fd, _T("%u (0x%x)\n"),
			    td->td_planarconfig, td->td_planarconfig);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_PAGENAME))
		_ftprintf(fd, _T("  Page Name: \"%s\"\n"), td->td_pagename);
	if (TIFFFieldSet(tif,FIELD_GROUP3OPTIONS)) {
		_ftprintf(fd, _T("  Group 3 Options:"));
		sep = _T(" ");
		if (td->td_group3options & GROUP3OPT_2DENCODING)
			_ftprintf(fd, _T("%s2-d encoding"), sep), sep = _T("+");
		if (td->td_group3options & GROUP3OPT_FILLBITS)
			_ftprintf(fd, _T("%sEOL padding"), sep), sep = _T("+");
		if (td->td_group3options & GROUP3OPT_UNCOMPRESSED)
			_ftprintf(fd, _T("%suncompressed data"), sep);
		_ftprintf(fd, _T(" (%lu = 0x%lx)\n"),
		    (DWORD) td->td_group3options,
		    (DWORD) td->td_group3options);
	}
	if (TIFFFieldSet(tif,FIELD_CLEANFAXDATA)) {
		_ftprintf(fd, _T("  Fax Data: "));
		switch (td->td_cleanfaxdata) {
		case CLEANFAXDATA_CLEAN:
			_ftprintf(fd, _T("clean\n"));
			break;
		case CLEANFAXDATA_REGENERATED:
			_ftprintf(fd, _T("receiver regenerated\n"));
			break;
		case CLEANFAXDATA_UNCLEAN:
			_ftprintf(fd, _T("uncorrected errors\n"));
			break;
		default:
			_ftprintf(fd, _T("(%u = 0x%x)\n"),
			    td->td_cleanfaxdata, td->td_cleanfaxdata);
			break;
		}
	}
	if (TIFFFieldSet(tif,FIELD_BADFAXLINES))
		_ftprintf(fd, _T("  Bad Fax Lines: %lu\n"),
		    (DWORD) td->td_badfaxlines);
	if (TIFFFieldSet(tif,FIELD_BADFAXRUN))
		_ftprintf(fd, _T("  Consecutive Bad Fax Lines: %u\n"),
		    td->td_badfaxrun);
	if (TIFFFieldSet(tif,FIELD_GROUP4OPTIONS)) {
		_ftprintf(fd, _T("  Group 4 Options:"));
		if (td->td_group4options & GROUP4OPT_UNCOMPRESSED)
			_ftprintf(fd, _T("uncompressed data"));
		_ftprintf(fd, _T(" (%lu = 0x%lx)\n"),
		    (DWORD) td->td_group4options,
		    (DWORD) td->td_group4options);
	}
	if (TIFFFieldSet(tif,FIELD_PAGENUMBER))
		_ftprintf(fd, _T("  Page Number: %u-%u\n"),
		    td->td_pagenumber[0], td->td_pagenumber[1]);
	if (TIFFFieldSet(tif,FIELD_COLORMAP)) {
		_ftprintf(fd, _T("  Color Map: "));
		if (flags & TIFFPRINT_COLORMAP) {
			_ftprintf(fd, _T("\n"));
			n = (WORD)(1<<td->td_bitspersample);
			for (i = 0; i < n; i++)
				_ftprintf(fd, _T("   %5d: %5u %5u %5u\n"),
				    i,
				    td->td_colormap[0][i],
				    td->td_colormap[1][i],
				    td->td_colormap[2][i]);
		} else
			_ftprintf(fd, _T("(present)\n"));
	}
#ifdef COLORIMETRY_SUPPORT
	if (TIFFFieldSet(tif,FIELD_WHITEPOINT))
		_ftprintf(fd, _T("  White Point: %g-%g\n"),
		    td->td_whitepoint[0], td->td_whitepoint[1]);
	if (TIFFFieldSet(tif,FIELD_PRIMARYCHROMAS))
		_ftprintf(fd, _T("  Primary Chromaticities: %g,%g %g,%g %g,%g\n"),
		    td->td_primarychromas[0], td->td_primarychromas[1],
		    td->td_primarychromas[2], td->td_primarychromas[3],
		    td->td_primarychromas[4], td->td_primarychromas[5]);
	if (TIFFFieldSet(tif,FIELD_REFBLACKWHITE)) {
		_ftprintf(fd, _T("  Reference Black/White:\n"));
		for (i = 0; i < td->td_samplesperpixel; i++)
			_ftprintf(fd, _T("    %2d: %5g %5g\n"),
			    i,
			    td->td_refblackwhite[2*i+0],
			    td->td_refblackwhite[2*i+1]);
	}
	if (TIFFFieldSet(tif,FIELD_TRANSFERFUNCTION)) {
		_ftprintf(fd, _T("  Transfer Function: "));
		if (flags & TIFFPRINT_CURVES) {
			_ftprintf(fd, _T("\n"));
			n = (WORD)(1<<td->td_bitspersample);
			for (i = 0; i < n; i++) {
				_ftprintf(fd, _T("    %2d: %5u"),
				    i, td->td_transferfunction[0][i]);
				for (j = 1; j < td->td_samplesperpixel; j++)
					_ftprintf(fd, _T(" %5u"),
					    td->td_transferfunction[j][i]);
				_puttc('\n', fd);
			}
		} else
			_ftprintf(fd, _T("(present)\n"));
	}
#endif
	if ((flags & TIFFPRINT_STRIPS) &&
	    TIFFFieldSet(tif,FIELD_STRIPOFFSETS)) {
		_ftprintf(fd, _T("  %u %s:\n"),
		    td->td_nstrips,
		    isTiled(tif) ? _T("Tiles") : _T("Strips"));
		for (i = 0; (long)i < td->td_nstrips; i++)
			_ftprintf(fd, _T("    %3d: [%8lu, %8lu]\n"),
			    i,
			    (DWORD) td->td_stripoffset[i],
			    (DWORD) td->td_stripbytecount[i]);
	}
}

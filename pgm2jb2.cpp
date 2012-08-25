#include <iostream>
//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
//C- | accompanied the software or you may obtain a copy of the license
//C- | from the Free Software Foundation at http://www.fsf.org .
//C- |
//C- | The computer code originally released by LizardTech under this
//C- | license and unmodified by other parties is deemed "the LIZARDTECH
//C- | ORIGINAL CODE."  Subject to any third party intellectual property
//C- | claims, LizardTech grants recipient a worldwide, royalty-free, 
//C- | non-exclusive license to make, use, sell, or otherwise dispose of 
//C- | the LIZARDTECH ORIGINAL CODE or of programs derived from the 
//C- | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU 
//C- | General Public License.   This grant only confers the right to 
//C- | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to 
//C- | the extent such infringement is reasonably necessary to enable 
//C- | recipient to make, have made, practice, sell, or otherwise dispose 
//C- | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to 
//C- | any greater extent that may be necessary to utilize further 
//C- | modifications or combinations.
//C- |
//C- | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
//C- | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- +------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

/** @name cjb2

  {\bf Synopsis}
  \begin{verbatim}
  cjb2 [options] <input-pbm-or-tiff>  <output-djvu>
  \end{verbatim}

  {\bf Description}

  File #"cjb2.cpp"# demonstrates a simple encoder for Bilevel DjVu Images.
  It is able to perform lossless encoding and limited lossy encoding.  Lots
  of lossy encoding refinements are missing from this simple implementation.
  Comments in the code suggest a few improvements.

  Options are:
  \begin{description}
  \item[-dpi xxx]     Specify image resolution (default 300).
  \item[-lossless]    Lossless compression (same as -losslevel 0, default).
  \item[-clean]       Quasi-lossless compression (same as -losslevel 1).
  \item[-lossy]       Lossy compression (same as -losslevel 100).
  \item[-losslevel n] Set loss level (0 to 200)
  \item[-verbose]     Display additional messages.
  \end{description}
  Encoding is lossless unless one or several lossy options are selected.
  The #dpi# argument mostly affects the cleaning thresholds.

  {\bf Bugs}

  This is not the full-fledged multipage DjVu compressor, but merely a free
  tool provided with the DjVu Reference Library as a demonstrative example.

  @memo
  Simple JB2 encoder.
  @author
  L\'eon Bottou <leonb@research.att.com>\\
  Paul Howard <pgh@research.att.com>\\
  Pascal Vincent <vincentp@iro.umontreal.ca>\\
  Ilya Mezhirov <ilya@mezhirov.mccme.ru>
  */
//@{
//@}


#include "djvulibre.h"

#include "jb2tune.h"

#include <locale.h>
#include <stddef.h>
#include <stdlib.h>
#if HAVE_TIFF
#include <tiffio.h>
#endif

// --------------------------------------------------
// UTILITIES
// --------------------------------------------------

#ifdef MIN
#undef MIN
#endif

    inline int 
MIN(int a, int b) 
{ 
    return ( a<b ?a :b); 
}

#ifdef MAX
#undef MAX
#endif

    inline int 
MAX(int a, int b) 
{ 
    return ( a>b ?a :b); 
}






// --------------------------------------------------
// CONNECTED COMPONENT ANALYSIS AND CLEANING
// --------------------------------------------------

// -- A run of black pixels
struct Run    
{ 
    int y;         // vertical coordinate
    short x1;      // first horizontal coordinate
    short x2;      // last horizontal coordinate
    int ccid;      // component id
};

// -- A component descriptor
struct CC    
{
    GRect bb;      // bounding box
    int npix;      // number of black pixels
    int nrun;      // number of runs
    int frun;      // first run in cc ordered array of runs
};


// -- An image composed of runs
class CCImage 
{
    public:
        int height;            // Height of the image in pixels
        int width;             // Width of the image in pixels
        GTArray<Run> runs;     // array of runs
        GTArray<CC>  ccs;      // Array of component descriptors
        int nregularccs;       // Number of regular ccs (set by merge_and_split_ccs)
        int largesize;         // CCs larger than that are special
        int smallsize;         // CCs smaller than that are special 
        int tinysize;          // CCs smaller than that may be removed 
        CCImage();
        void init(int width, int height, int dpi);
        void add_single_run(int y, int x1, int x2, int ccid=0);
        void add_bitmap_runs(const GBitmap &bm, int offx=0, int offy=0, int ccid=0);
        GP<GBitmap> get_bitmap_for_cc(int ccid) const;
        GP<JB2Image> get_jb2image() const;
        void make_ccids_by_analysis();
        void make_ccs_from_ccids();
        void erase_tiny_ccs();
        void merge_and_split_ccs();
        void sort_in_reading_order(); 
};


// -- Compares runs
    static inline bool
operator <= (const Run &a, const Run &b)
{
    return (a.y<b.y) || (a.y==b.y && a.x1<=b.x1);
}


// -- Constructs CCImage and provide defaults
    CCImage::CCImage()
: height(0), width(0), nregularccs(0)
{
}

    void
CCImage::init(int w, int h, int dpi)
{
    runs.empty();
    ccs.empty();
    height = h;
    width = w;
    nregularccs = 0;
    dpi = MAX(200, MIN(900, dpi));
    largesize = MIN( 500, MAX(64, dpi));
    smallsize = MAX(2, dpi/150);
    tinysize = MAX(0, dpi*dpi/20000 - 1) * 2;
    std::cerr << "tiny: " << tinysize << '\n';
}


// -- Adds a run to the CCImage
    inline void 
CCImage::add_single_run(int y, int x1, int x2, int ccid)
{
    int index = runs.hbound();
    runs.touch(++index);
    Run& run = runs[index];
    run.y = y;
    run.x1 = x1;
    run.x2 = x2;
    run.ccid = ccid;
}


// -- Adds runs extracted from a bitmap
    void 
CCImage::add_bitmap_runs(const GBitmap &bm, int offx, int offy, int ccid)
{
    // Iterate over rows
    for (unsigned int y=0; y<bm.rows(); y++)
    {
        const unsigned char *row = bm[y];
        int w = bm.columns();
        int x = 0;
        // Iterate over runs
        while (x < w)
        {
            while (x < w  && !row[x]) x++;
            if (x < w)
            {
                int x1 = x;
                while (x < w && row[x]) x++;
                add_single_run(offy+y, offx+x1, offx+x-1, ccid);
            }
        }
    }
}


// -- Performs connected component analysis
    void
CCImage::make_ccids_by_analysis()
{
    // Sort runs
    runs.sort();
    // Single Pass Connected Component Analysis (with unodes)
    int n;
    int p=0;
    GTArray<int> umap;
    for (n=0; n<=runs.hbound(); n++)
    {
        int y = runs[n].y;
        int x1 = runs[n].x1 - 1;
        int x2 = runs[n].x2 + 1;
        int id = (umap.hbound() + 1);
        // iterate over previous line runs
        for(;runs[p].y < y-1;p++);
        for(;(runs[p].y < y) && (runs[p].x1 <= x2);p++ )
        {
            if ( runs[p].x2 >= x1 )
            {
                // previous run touches current run
                int oid = runs[p].ccid;
                while (umap[oid] < oid)
                    oid = umap[oid];
                if ((int)id > umap.hbound()) {
                    id = oid;
                } else if (id < oid) {
                    umap[oid] = id;
                } else {
                    umap[id] = oid;
                    id = oid;
                }
                // freshen previous run id
                runs[p].ccid = id;
                // stop if previous run goes past current run
                if (runs[p].x2 >= x2)
                    break;
            }
        }
        // create new entry in umap
        runs[n].ccid = id;
        if (id > umap.hbound())
        {
            umap.touch(id);
            umap[id] = id;
        }
    }
    // Update umap and ccid
    for (n=0; n<=runs.hbound(); n++)
    {
        Run &run = runs[n];
        int ccid = run.ccid;
        while (umap[ccid] < ccid)
        {
            ccid = umap[ccid];
        }
        umap[run.ccid] = ccid;
        run.ccid = ccid;
    }
}


// -- Constructs the ``ccs'' array from run's ccids.
    void
CCImage::make_ccs_from_ccids()
{
    int n;
    Run *pruns = runs;
    // Find maximal ccid
    int maxccid = nregularccs-1;
    for (n=0; n<=runs.hbound(); n++)
        if (pruns[n].ccid > maxccid)
            maxccid = runs[n].ccid;
    // Renumber ccs 
    GTArray<int> armap(0,maxccid);
    int *rmap = armap;
    for (n=0; n<=maxccid; n++)
        armap[n] = -1;
    for (n=0; n<=runs.hbound(); n++)
        if (pruns[n].ccid >= 0)
            rmap[ pruns[n].ccid ] = 1;
    int nid = 0;
    for (n=0; n<=maxccid; n++)
        if (rmap[n] > 0)
            rmap[n] = nid++;

    // Adjust nregularccs (since ccs are renumbered)
    while (nregularccs>0 && rmap[nregularccs-1]<0)
        nregularccs -= 1;
    if (nregularccs>0)
        nregularccs = 1 + rmap[nregularccs-1];

    // Prepare cc descriptors
    ccs.resize(0,nid-1);
    for (n=0; n<nid; n++)
        ccs[n].nrun = 0;

    // Relabel runs
    for (n=0; n<=runs.hbound(); n++)
    {
        Run &run = pruns[n];
        if (run.ccid < 0) continue;  // runs with negative ccids are destroyed
        int oldccid = run.ccid;
        int newccid = rmap[oldccid];
        CC &cc = ccs[newccid];
        run.ccid = newccid;
        cc.nrun += 1;
    }

    // Compute positions for runs of cc
    int frun = 0;
    for (n=0; n<nid; n++) 
    {
        ccs[n].frun = rmap[n] = frun;
        frun += ccs[n].nrun;
    }

    // Copy runs
    GTArray<Run> rtmp;
    rtmp.steal(runs);
    Run *ptmp = rtmp;
    runs.resize(0,frun-1);
    pruns = runs;
    for (n=0; n<=rtmp.hbound(); n++)
    {
        int id = ptmp[n].ccid;
        if (id < 0) continue;
        int pos = rmap[id]++;
        pruns[pos] = ptmp[n];
    }

    // Finalize ccs
    for (n=0; n<nid; n++)
    {
        CC &cc = ccs[n];
        int npix = 0;
        runs.sort(cc.frun, cc.frun+cc.nrun-1);
        Run *run = &runs[cc.frun];
        int xmin = run->x1;
        int xmax = run->x2;
        int ymin = run->y;
        int ymax = run->y;
        for (int i=0; i<cc.nrun; i++, run++)
        {
            if (run->x1 < xmin)  xmin = run->x1;
            if (run->x2 > xmax)  xmax = run->x2;
            if (run->y  < ymin)  ymin = run->y;
            if (run->y  > ymax)  ymax = run->y;
            npix += run->x2 - run->x1 + 1;
        }
        cc.npix = npix;
        cc.bb.xmin = xmin;
        cc.bb.ymin = ymin;
        cc.bb.xmax = xmax + 1;
        cc.bb.ymax = ymax + 1;
    }
}


// Removes ccs which are too small.
    void
CCImage::erase_tiny_ccs()
{
    // ISSUE: HALFTONE DETECTION
    // We should not remove tiny ccs if they are part of a halftone pattern...
    for (int i=0; i<ccs.size(); i++)
    {
        CC& cc = ccs[i];
        if (cc.npix <= tinysize)
        {
            // Mark cc to be erased
            Run *r = &runs[cc.frun];
            int nr = cc.nrun;
            cc.nrun = 0;
            cc.npix = 0;
            while (--nr >= 0)
                (r++)->ccid = -1;
        }
    }
}


// -- Merges small ccs and split large ccs
    void
CCImage::merge_and_split_ccs()
{
    int ncc = ccs.size();
    int nruns = runs.size();
    int splitsize = largesize;
    if (ncc <= 0) return;
    // Grid of special components
    int gridwidth = (width+splitsize-1)/splitsize;
    nregularccs = ncc;
    // Set the correct ccids for the runs
    for (int ccid=0; ccid<ncc; ccid++)
    {
        CC* cc = &ccs[ccid];
        if (cc->nrun <= 0) continue;
        int ccheight = cc->bb.height();
        int ccwidth = cc->bb.width();
        if (ccheight<=smallsize && ccwidth<=smallsize)
        {
            int gridi = (cc->bb.ymin+cc->bb.ymax)/splitsize/2;
            int gridj = (cc->bb.xmin+cc->bb.xmax)/splitsize/2;
            int newccid = ncc + gridi*gridwidth + gridj;
            for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
                runs[runid].ccid = newccid;
        }
        else if (ccheight>=largesize || ccwidth>=largesize)
        {
            for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
            {
                Run& r = runs[runid];
                int y = r.y;
                int x_start = r.x1;
                int x_end = r.x2;
                int gridi = y/splitsize;
                int gridj_start = x_start/splitsize;
                int gridj_end = x_end/splitsize;
                int gridj_span = gridj_end-gridj_start;
                int newccid = ncc + gridi*gridwidth + gridj_start;
                if (! gridj_span)
                {
                    r.ccid = newccid;
                }
                else // gridj_span>0
                {
                    // truncate the current run 
                    r.ccid = newccid++;
                    int x = (gridj_start+1)*splitsize;
                    r.x2 = x-1;
                    runs.touch(nruns+gridj_span-1);
                    // append additional runs to the runs array
                    for(int gridj=gridj_start+1; gridj<gridj_end; gridj++)
                    {
                        Run& newrun = runs[nruns++];
                        newrun.y = y;
                        newrun.x1 = x;
                        x += splitsize;
                        newrun.x2 = x-1;
                        newrun.ccid = newccid++;
                    }
                    // append last run to the run array
                    Run& newrun = runs[nruns++];
                    newrun.y = y;
                    newrun.x1 = x;
                    newrun.x2 = x_end;
                    newrun.ccid = newccid++;                      
                }
            }
        }
    }
    // Recompute cc descriptors
    make_ccs_from_ccids();
}


// -- Helps sorting cc
    static int 
top_edges_descending (const void *pa, const void *pb)
{
    if (((CC*) pa)->bb.ymax != ((CC*) pb)->bb.ymax)
        return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
    if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
        return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
    return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
    static int 
left_edges_ascending (const void *pa, const void *pb)
{
    if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
        return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
    if (((CC*) pb)->bb.ymax != ((CC*) pa)->bb.ymax)
        return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
    return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
    static int 
integer_ascending (const void *pa, const void *pb)
{
    return ( *(int*)pb - *(int*)pa );
}


// -- Sort ccs in approximate reading order
    void 
CCImage::sort_in_reading_order()
{
    if (nregularccs<2) return;
    CC *ccarray = new CC[nregularccs];
    // Copy existing ccarray (but segregate special ccs)
    int ccid;
    for(ccid=0; ccid<nregularccs; ccid++)
        ccarray[ccid] = ccs[ccid];
    // Sort the ccarray list into top-to-bottom order.
    qsort (ccarray, nregularccs, sizeof(CC), top_edges_descending);
    // Subdivide the ccarray list roughly into text lines [LYB]
    // - Determine maximal top deviation
    int maxtopchange = width / 40;
    if (maxtopchange < 32) 
        maxtopchange = 32;
    // - Loop until processing all ccs
    int ccno = 0;
    int *bottoms = new int[nregularccs];
    while (ccno < nregularccs)
    {
        // - Gather first line approximation
        int nccno;
        int sublist_top = ccarray[ccno].bb.ymax-1;
        int sublist_bottom = ccarray[ccno].bb.ymin;
        for (nccno=ccno; nccno < nregularccs; nccno++)
        {
            if (ccarray[nccno].bb.ymax-1 < sublist_bottom) break;
            if (ccarray[nccno].bb.ymax-1 < sublist_top - maxtopchange) break;
            int bottom = ccarray[nccno].bb.ymin;
            bottoms[nccno-ccno] = bottom;
            if (bottom < sublist_bottom)
                sublist_bottom = bottom;
        }
        // - If more than one candidate cc for the line
        if (nccno > ccno + 1)
        {
            // - Compute median bottom
            qsort(bottoms, nccno-ccno, sizeof(int), integer_ascending);
            int bottom = bottoms[ (nccno-ccno-1)/2 ];
            // - Compose final line
            for (nccno=ccno; nccno < nregularccs; nccno++)
                if (ccarray[nccno].bb.ymax-1 < bottom)
                    break;
            // - Sort final line
            qsort (ccarray+ccno, nccno-ccno, sizeof(CC), left_edges_ascending);
        }
        // - Next line
        ccno = nccno;
    }
    // Copy ccarray back and renumber the runs
    for(ccid=0; ccid<nregularccs; ccid++)
    {
        CC& cc = ccarray[ccid];
        ccs[ccid] = cc;
        for(int r=cc.frun; r<cc.frun+cc.nrun; r++)
            runs[r].ccid = ccid;
    }
    // Free memory
    delete [] bottoms;
    delete[] ccarray;
}


// -- Creates a bitmap for a particular component
GP<GBitmap>   
CCImage::get_bitmap_for_cc(const int ccid) const
{
    const CC &cc = ccs[ccid];
    const GRect &bb = cc.bb;
    GP<GBitmap> bits = GBitmap::create(bb.height(), bb.width());
    const Run *prun = & runs[(int)cc.frun];
    for (int i=0; i<cc.nrun; i++,prun++)
    {
        if (prun->y<bb.ymin || prun->y>=bb.ymax)
            G_THROW("Internal error (y bounds)");
        if (prun->x1<bb.xmin || prun->x2>=bb.xmax)
            G_THROW("Internal error (x bounds)");
        unsigned char *row = (*bits)[prun->y - bb.ymin];
        for (int x=prun->x1; x<=prun->x2; x++)
            row[x - bb.xmin] = 1;
    }
    return bits;
}


// -- Creates a JB2Image with the remaining components
GP<JB2Image> 
CCImage::get_jb2image() const
{
    GP<JB2Image> jimg = JB2Image::create();
    jimg->set_dimension(width, height);
    if (runs.hbound() < 0)
        return jimg;
    if (ccs.hbound() < 0)
        G_THROW("Must first perform a cc analysis");
    // Iterate over CCs
    for (int ccid=0; ccid<=ccs.hbound(); ccid++)
    {
        JB2Shape shape;
        JB2Blit  blit;
        shape.parent = -1;
        shape.bits = get_bitmap_for_cc(ccid);
        shape.userdata = 0;
        if (ccid >= nregularccs)
            shape.userdata |= JB2SHAPE_SPECIAL;
        blit.shapeno = jimg->add_shape(shape);
        blit.left = ccs[ccid].bb.xmin;
        blit.bottom = ccs[ccid].bb.ymin;
        jimg->add_blit(blit);
        shape.bits->compress();
    }
    // Return
    return jimg;
}

GP<JB2Image> pbm2jb2(const GP<GBitmap> &image, int losslevel = 0, int dpi = 300) {
    CCImage rimg;
    rimg.init(image->columns(), image->rows(), dpi);
    rimg.add_bitmap_runs(*image); 
    // Component analysis
    rimg.make_ccids_by_analysis(); // obtain ccids
    rimg.make_ccs_from_ccids();    // compute cc descriptors

    if (losslevel > 0) {
        rimg.erase_tiny_ccs();       // clean
    }
    rimg.merge_and_split_ccs();    // reorganize weird ccs
    rimg.sort_in_reading_order();  // sort cc descriptors

    // Pattern matching
    GP<JB2Image> jimg = rimg.get_jb2image();          // get ``raw'' jb2image
    rimg.runs.empty();                                // save memory
    rimg.ccs.empty();                                 // save memory
    if (losslevel>1)
        tune_jb2image_lossy(jimg, dpi, losslevel);
    else
        tune_jb2image_lossless(jimg);
    return jimg;
}

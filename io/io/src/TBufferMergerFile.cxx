// @(#)root/io:$Id$
// Author: Philippe Canal, Witold Pokorski, and Guilherme Amadio

/*************************************************************************
 * Copyright (C) 1995-2017, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "ROOT/TBufferMerger.hxx"

#include "TArrayC.h"
#include "TBufferFile.h"

namespace ROOT {
namespace Experimental {

TBufferMergerFile::TBufferMergerFile(TBufferMerger &m)
   : TMemFile(m.fFile->GetName(), "RECREATE", "", m.fFile->GetCompressionLevel()), fMerger(m), fClassSent(nullptr)
{
}

TBufferMergerFile::~TBufferMergerFile()
{
}

Int_t TBufferMergerFile::Write(const char *name, Int_t opt, Int_t bufsize)
{
   Int_t nbytes = TMemFile::Write(name, opt, bufsize);

   if (nbytes) {
      TBufferFile *fBuffer = new TBufferFile(TBuffer::kWrite);

      fBuffer->WriteTString(GetName());
      fBuffer->WriteLong64(GetEND());
      CopyTo(*fBuffer);

      fMerger.Push(fBuffer);

      // Record StreamerInfo sent to the server
      Int_t isize = fClassIndex->GetSize();
      if (!fClassSent) {
         fClassSent.reset(new TArrayC(isize));
      } else {
         if (isize > fClassSent->GetSize()) {
            fClassSent->Set(isize);
         }
      }
      for (Int_t c = 0; c < isize; ++c) {
         if (fClassIndex->fArray[c]) {
            fClassSent->fArray[c] = 1;
         }
      }
      ResetAfterMerge(0);
   }
   return nbytes;
}

void TBufferMergerFile::WriteStreamerInfo()
{
   if (!fWritable) return;
   if (!fClassIndex) return;
   // no need to update the index if no new classes added to the file
   if (fClassIndex->fArray[0] == 0) return;

   // clear fClassIndex for anything we already sent.
   if (fClassSent) {
      Int_t isize = fClassIndex->GetSize();
      Int_t ssize = fClassSent->GetSize();
      for (Int_t c = 0; c < isize && c < ssize; ++c) {
         if (fClassSent->fArray[c]) {
            fClassIndex->fArray[c] = 0;
         }
      }
   }

   TMemFile::WriteStreamerInfo();
}

} // namespace Experimental
} // namespace ROOT

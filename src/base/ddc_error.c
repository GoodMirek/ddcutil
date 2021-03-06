/* ddc_error.c
 *
 * Created on: Oct 22, 2017
 *     Author: rock
 *
 * <copyright>
 * Copyright (C) 2014-2015 Sanford Rockowitz <rockowitz@minsoft.com>
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * </endcopyright>
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/report_util.h"

#include "ddc_errno.h"
#include "retry_history.h"
#include "status_code_mgt.h"

#include "ddc_error.h"


#define VALID_DDC_ERROR_PTR(ptr) \
   assert(ptr); \
   assert(memcmp(ptr->marker, DDC_ERROR_MARKER, 4) == 0);


void ddc_error_free(Ddc_Error * erec){
   VALID_DDC_ERROR_PTR(erec);
   for (int ndx = 0; ndx < erec->cause_ct; ndx++) {
      ddc_error_free(erec->causes[ndx]);
   }
   free(erec->func);
   erec->marker[3] = 'x';
   free(erec);
}


Ddc_Error *  ddc_error_new(Public_Status_Code psc, const char * func) {
   Ddc_Error * erec = calloc(1, sizeof(Ddc_Error));
   memcpy(erec->marker, DDC_ERROR_MARKER, 4);
   erec->psc = psc;
   erec->func = strdup(func);   // strdup to avoid constness warning, must free
   return erec;
}

Ddc_Error * ddc_error_new_with_cause(Public_Status_Code psc, Ddc_Error * cause, char * func) {
   Ddc_Error * erec = ddc_error_new(psc, func);
   erec->causes[0] = cause;
   erec->cause_ct  = 1;
   return erec;
}

Ddc_Error * ddc_error_new_chained(Ddc_Error * cause, char * func) {
   Ddc_Error * erec = ddc_error_new_with_cause(cause->psc, cause, func);
   return erec;
}

Ddc_Error * ddc_error_new_retries(
      Public_Status_Code *  status_codes,
      int                   status_code_ct,
      const char *          called_func,
      const char *          func)
{
   Ddc_Error * result = ddc_error_new(DDCRC_RETRIES, func);
   for (int ndx = 0; ndx < status_code_ct; ndx++) {
      Ddc_Error * cause = ddc_error_new(status_codes[ndx],called_func);
      ddc_error_add_cause(result, cause);
   }
   return result;
}

void ddc_error_add_cause(Ddc_Error * parent, Ddc_Error * cause) {
   VALID_DDC_ERROR_PTR(parent);
   VALID_DDC_ERROR_PTR(cause);

   // assert(parent);
   // assert(memcpy(parent->marker, DDC_ERROR_MARKER, 4) == 0);
   assert(parent->cause_ct < MAX_MAX_TRIES);

   // assert(cause);
   // assert(memcpy(cause->marker, DDC_ERROR_MARKER, 4) == 0);

   parent->causes[parent->cause_ct++] = cause;
}

void ddc_error_set_status(Ddc_Error * erec, Public_Status_Code psc) {
   VALID_DDC_ERROR_PTR(erec);
   erec->psc = psc;
}

char * ddc_error_causes_string(Ddc_Error * erec) {
   // return strdup("unimplemented");
   // *** Temporary hacked up implementation ***
   // TODO: Reimplement
   Retry_History * hist = ddc_error_to_new_retry_history(erec);
   char * result = retry_history_string(hist);
   free(hist);
   return result;
}

void report_ddc_error(Ddc_Error * erec, int depth) {
   int d1 = depth+1;

   // rpt_vstring(depth, "Status code: %s", psc_desc(erec->psc));
   // rpt_vstring(depth, "Location: %s", (erec->func) ? erec->func : "not set");
   rpt_vstring(depth, "%s: status=%s",
         (erec->func) ? erec->func : "not set",
                      psc_desc(erec->psc)


   );
   if (erec->cause_ct > 0) {
      rpt_vstring(depth, "Caused by: ");
      for (int ndx = 0; ndx < erec->cause_ct; ndx++) {
         report_ddc_error(erec->causes[ndx], d1);
      }
   }
}

//
// Transitional functions
//


void ddc_error_fill_retry_history(Ddc_Error * erec, Retry_History * hist) {
   VALID_DDC_ERROR_PTR(erec);
   assert(erec->psc == DDCRC_RETRIES);

   for (int ndx = 0; ndx < erec->cause_ct; ndx++) {
      retry_history_add(hist, erec->causes[ndx]->psc);
   }
}


Retry_History * ddc_error_to_new_retry_history(Ddc_Error * erec) {
   VALID_DDC_ERROR_PTR(erec);
   assert(erec->psc == DDCRC_RETRIES);

   Retry_History * hist = retry_history_new();
   ddc_error_fill_retry_history(erec, hist);

   return hist;
}



Ddc_Error * ddc_error_from_retry_history(Retry_History * hist, char * func) {
   assert(hist);
   assert(memcmp(hist->marker, RETRY_HISTORY_MARKER, 4) == 0);

   Ddc_Error * erec = ddc_error_new(DDCRC_RETRIES, func);
   for (int ndx = 0; ndx < hist->ct; ndx++) {
      Ddc_Error * cause = ddc_error_new(hist->psc[ndx], "dummy");
      erec->causes[ndx] = cause;
   }
   return erec;
}





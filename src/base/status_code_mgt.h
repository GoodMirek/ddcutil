/* status_code_mgt.h
 *
 * <copyright>
 * Copyright (C) 2014-2016 Sanford Rockowitz <rockowitz@minsoft.com>
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

/** \file
 * Status Code Management
 */

#ifndef STATUS_CODE_MGT_H_
#define STATUS_CODE_MGT_H_

/** \cond */
#include <stdbool.h>
/** \endcond */


// Called from the mainline to perform initialization
void init_status_code_mgt();

/** Describes one status code.
 *
 * @remark
 * Code could be simplified by using a #Value_Name_Title table
 * instead, but left as is because we might want to add additional
 * fields.
 */
typedef
struct {
   int    code;
   char * name;
   char * description;
   // idea: add flags for NOT_AN_ERROR, DERIVED ?
} Status_Code_Info;

// debugging function:
void report_status_code_info(Status_Code_Info * pdesc);


// For distinguishing types of return codes.
// C does not enforce type checking, but useful for documentation
// trying different styles for readability, consistency w standards

/** Fully modulated status code */
typedef int Global_Status_Code;
// Global_Status_Code subranges:

// typedef int Global_Status_ADL;    // subrange of Global_Status_Code containing modulated ADL return codes
typedef int Global_Status_DDC;    // subrange of Global_Status_Code containing (always modulated) DDC return codes
typedef int Global_Status_Errno;  // subrange of Global_Status_Code containing modulated Linux return codes

typedef int Modulated_Status_ADL; ///< modulated ADL return codes only, same as Global_Status_ADL

// typedef int Rc_Raw_DDC_t;
typedef int Base_Status_ADL;
typedef int Base_Status_Errno;    //< negative Linux errno values, unmodulated

// would make it simpler for low level I2C functions to be incorporated into sample code
typedef int Base_Status_Errno_DDC;   ///< union(Base_Status_Errno, modulated DDC)

// unused
//typedef int Base_Status_ADL_DDC;     // union(Base_Status_ADL, Global_Status_DDC)

typedef int Public_Status_Code;      // union(unmodulated Linux codes, modulated ADL codes, (always modulaed) DDC return codes

/** typedef of function used to return a #Status_Code_Info for a status code */
typedef
Status_Code_Info * (*Retcode_Description_Finder)(int rc);

typedef
bool (*Retcode_Number_Finder)(const char * name, Global_Status_Code * p_number);



Global_Status_Code modulate_base_errno_ddc_to_global(Base_Status_Errno_DDC rc);

//
// Status codes ranges
//
#define RCRANGE_BASE_START      0
#define RCRANGE_BASE_MAX      999
#define RCRANGE_ERRNO_START  1000
#define RCRANGE_ERRNO_MAX    1999
#define RCRANGE_ADL_START    2000
#define RCRANGE_ADL_MAX      2999
#define RCRANGE_DDC_START    3000
#define RCRANGE_DDC_MAX      3999

/** Status code range identifiers
 *
 * @remark
 * - must be kept consistent with table in status_code_mgt.c
 * - should RR_BASE be in this enum?
 */
typedef enum {
          RR_BASE,     ///< indicates unmodulated status code
          RR_ERRNO,    ///< range id for modulated Linux errno values
          RR_ADL,      ///< range id for modulated ADL error codes
          RR_DDC       ///< range id for modulated ddcutil-specific error codes
} Retcode_Range_Id;

#ifdef OLD
void register_retcode_desc_finder(
        Retcode_Range_Id           id,
        Retcode_Description_Finder finder_func,
        bool                       finder_arg_is_modulated);
#endif

int modulate_rc(int unmodulated_rc, Retcode_Range_Id range_id);
int demodulate_rc(int modulated_rc, Retcode_Range_Id range_id);
Retcode_Range_Id get_modulation(int rc);
// int demodulate_any_rc(int modulated_rc);   // unimplemented

Public_Status_Code global_to_public_status_code(Global_Status_Code gsc);
Global_Status_Code public_to_global_status_code(Public_Status_Code);

Status_Code_Info * find_global_status_code_info(Global_Status_Code rc);

// Returns status code description:
char * gsc_desc(Global_Status_Code rc);   // must be freed after use
char * psc_desc(Public_Status_Code rc);
char * gsc_name(Global_Status_Code status_code);   // do not free after use

bool gsc_name_to_unmodulated_number(const char * status_code_name, int * p_error_number);
bool gsc_name_to_modulated_number(const char * status_code_name, Global_Status_Code * p_error_number);


#endif /* STATUS_CODE_MGT_H_ */

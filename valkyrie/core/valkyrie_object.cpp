/* --------------------------------------------------------------------- 
 * Implementation of class Valkyrie                  valkyrie_object.cpp
 * Valkyrie-specific options / flags / fns
 * ---------------------------------------------------------------------
 * This file is part of Valkyrie, a front-end for Valgrind
 * Copyright (c) 2000-2006, OpenWorks LLP <info@open-works.co.uk>
 * This program is released under the terms of the GNU GPL v.2
 * See the file LICENSE.GPL for the full license details.
 */

#include "valkyrie_object.h"
#include "config.h"            // PACKAGE_VERSION, BIN_VALGRIND
#include "valgrind_object.h"
#include "tool_object.h"
#include "vk_config.h"
#include "vk_messages.h"
#include "vk_utils.h"          // vk_assert(), vk_strcmp(), vkPrint()
#include "html_urls.h"
#include "vk_option.h"         // PERROR* and friends 

#include <stdio.h>
#include <stdlib.h>            /* exit */

#include <qapplication.h>



/* class Valkyrie --------------------------------------------------- */
Valkyrie:: ~Valkyrie()
{
   if (m_valgrind != 0) {
      delete m_valgrind;
      m_valgrind = 0;
   }
}


Valkyrie::Valkyrie()
   : VkObject( "Valkyrie", "Valkyrie", Qt::Key_unknown, VkObject::ID_VALKYRIE ) 
{
   /* init vars */
   m_startRunState = VkRunState::STOPPED;

   /* init valgrind */
   m_valgrind = new Valgrind();

   /* init tools */
   initToolObjects();


   addOpt( HELP,         VkOPTION::ARG_NONE,   VkOPTION::WDG_NONE,
           "valkyrie",   'h',                  "help", 
           "",           "",                   "", 
           "",           "show this help message and exit", urlNone );
   addOpt( OPT_VERSION,  VkOPTION::ARG_NONE,   VkOPTION::WDG_NONE,
           "valkyrie",   'v',                  "version", 
           "",           "",                   "", 
           "",           "display version information and exit", urlNone );
   addOpt( VGHELP,       VkOPTION::ARG_NONE,   VkOPTION::WDG_NONE,
           "valkyrie",   'V',                  "valgrind-opts", 
           "",           "",                   "", 
           "",           "Show valgrind options too, and exit", urlNone );
   addOpt( TOOLTIP,      VkOPTION::NOT_POPT,   VkOPTION::WDG_CHECK, 
           "valkyrie",  '\0',                  "show-tooltips", 
           "",           "true|false",         "true", 
           "Show tooltips",      "",           urlValkyrie::toolTips );
   addOpt( PALETTE,      VkOPTION::NOT_POPT,   VkOPTION::WDG_CHECK, 
           "valkyrie",   '\0',                 "use-vk-palette", 
           "",           "true|false",         "true", 
           "Use valkyrie's palette",   "",     urlValkyrie::palette );
   addOpt( ICONTXT,      VkOPTION::NOT_POPT,   VkOPTION::WDG_CHECK, 
           "valkyrie",   '\0',                 "show-butt-text", 
           "",           "true|false",         "true", 
           "Show toolbar text labels",  "",    urlValkyrie::toolLabels );
   addOpt( FNT_GEN_SYS,  VkOPTION::NOT_POPT,   VkOPTION::WDG_CHECK,
           "valkyrie",   '\0',                 "font-gen-sys", 
           "",           "true|false",         "true", 
           "Use system default", "",           urlValkyrie::userFontGen );
   addOpt( FNT_GEN_USR,  VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "font-gen-user", 
           "",           "",                   "Luxi Sans,10,-1,5,50,0,0,0,0,0", 
           "Choose Font",   "",                urlValkyrie::userFontGen );
   addOpt( FNT_TOOL_USR, VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "font-tool-user", 
           "",           "",                   "Misc Fixed,11,-1,5,50,0,0,0,0,0", 
           "Choose Font",   "",                urlValkyrie::userFontTool );
   addOpt( SRC_EDITOR,   VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "src-editor", 
           "",           "",                   BIN_EDITOR, 
           "Src Editor:",   "",                urlValkyrie::srcEditor );
   addOpt( SRC_LINES,    VkOPTION::NOT_POPT,   VkOPTION::WDG_SPINBOX, 
           "valkyrie",   '\0',                 "src-lines",
           "",           "1|10",               "2", 
           "Extra lines shown above/below the target line:", "",
           urlValkyrie::srcLines );
   addOpt( BROWSER,      VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "browser", 
           "",           "",                   "", 
           "Browser:",   "",                   urlValkyrie::browser );
   addOpt( DFLT_LOGDIR,  VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "default-logdir", 
           "",           "",                   VK_LOGS_DIR, 
           "Log Dir:",   "",                   "" );

   /* path to valgrind executable (maybe found by configure) */
   addOpt( VG_EXEC,      VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT, 
           "valkyrie",   '\0',                 "vg-exec",
           "",           "",                   BIN_VALGRIND,
           "Valgrind:",  "",                   urlValkyrie::vgDir );
   addOpt( BINARY,       VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT,
           "valkyrie",   '\0',                 "binary", 
           "",           "",                   "", 
           "Binary:",    "",                   urlValkyrie::binary );
   addOpt( BIN_FLAGS,    VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT,
           "valkyrie",   '\0',                 "binary-flags", 
           "",           "",                   "", 
           "Binary flags:", "",                urlValkyrie::binFlags );
   addOpt( VIEW_LOG,     VkOPTION::ARG_STRING, VkOPTION::WDG_NONE, 
           "valkyrie",   '\0',                 "view-log", 
           "<file>",     "",                   "",
           "View logfile:", "parse and view a valgrind logfile",
           urlNone );

   /* path to vk_logmerge executable (found by configure) */
   addOpt( MERGE_EXEC,   VkOPTION::NOT_POPT,   VkOPTION::WDG_LEDIT,
           "valkyrie",   '\0',                 "merge-exec",
           "",           "",                   "",
           "Log Merger:", "",                  urlNone );
   addOpt( MERGE_LOGS,   VkOPTION::ARG_STRING, VkOPTION::WDG_NONE, 
           "valkyrie",   '\0',                 "merge", 
           "<loglist>",  "",                   "",
           "View logfiles:", "merge multiple logfiles, discarding duplicate errors",
           urlNone );
}


/* check argval for this option, updating if necessary.
   called by parseCmdArgs() and gui option pages -------------------- */
int Valkyrie::checkOptArg( int optid, QString& argval )
{ 
   vk_assert( optid >= 0 && optid < NUM_OPTS );

   int errval = PARSED_OK;
   // Option* opt = findOption( optid );

   switch ( (Valkyrie::vkOpts)optid ) {

      /* these options are _only_ set via the gui, and are either (a)
         limited to a set of available values, or (b) have already been
         checked, so no need to re-check them. */
   case TOOLTIP:
   case PALETTE:
   case ICONTXT:
   case FNT_GEN_SYS:
   case FNT_GEN_USR:
   case FNT_TOOL_USR:
   case SRC_LINES:
   case BROWSER:
      return errval;
      break;

   case DFLT_LOGDIR: {
      /* see if we have an dir with rx permissions */
      (void) dirCheck( &errval, argval, true, true );
   } break;

   case SRC_EDITOR: {
      QString ed_file = QStringList::split(" ", argval).first();
      QString ed = binaryCheck( &errval, ed_file );
      argval.replace( ed_file, ed );
   } break;

   case MERGE_EXEC:
      argval = binaryCheck( &errval, argval );
      break;

   case VG_EXEC: {
      /* see if we have an executable with rx permissions */
      argval = binaryCheck( &errval, argval );
      if ( errval != PARSED_OK )
         return errval;

      /* check the version */
      QString cmd = argval + " --version 2>&1";
      FILE* fp;
      char line[50];
      fp = popen( (const char*)cmd, "r" );
      if ( !fp ) {
         pclose(fp);
         return PERROR_BADFILE;
      }
      fgets( line, sizeof(line), fp );
      pclose(fp);

      QString vg_version = QString(line).simplifyWhiteSpace();
      if ( !vg_version.startsWith("valgrind") )
         return PERROR_BADFILE;
      /* compare with minimum req'd version: */
      if ( str2hex(vg_version) <  str2hex("3.0.0") )
         return PERROR_BADVERSION;
      /* looking good... */
   } break;

   case VIEW_LOG:
      argval = fileCheck( &errval, argval, true, false );
      if ( errval == PARSED_OK )
         m_startRunState = VkRunState::TOOL1;
      break;

      /* expects a single file which contains a list of
         logfiles-to-be-merged, each on a separate line, with a minimum
         of two logfiles. Validating each file is done at merge-time, as
         we will then skip any files which we can't read. */
   case MERGE_LOGS:
      argval = fileCheck( &errval, argval, true, false );
      if ( errval == PARSED_OK )
         m_startRunState = VkRunState::TOOL2;
      break;

   case BINARY:
      if ( !argval.isEmpty() )
         argval = binaryCheck( &errval, argval );

      if ( errval == PARSED_OK )
         m_startRunState = VkRunState::VALGRIND;
      break;

   /* can't really test this */
   case BIN_FLAGS:
      break;

   /* ignore these opts */
   case HELP:
   case VGHELP:
   case OPT_VERSION:
      break;

   default:
      vk_assert_never_reached();
   }

   return errval; 
}


/* Gather all config entries that hold persistent data
   - basically all options with an associated option widget.
   Called from VkConfig::mkConfigFile() when we need to create the
   valkyrierc file for the very first time. */
QString Valkyrie::configEntries()
{
   QString cfgEntry = "\n[" + name() + "]\n";
   for ( Option* opt = m_optList.first(); opt; opt = m_optList.next() ) {

      /* Don't create config entries for these options:
         They don't hold persistent data, and have no associated option
         widget */
      if (opt->m_key == Valkyrie::HELP       ) continue;
      if (opt->m_key == Valkyrie::VGHELP     ) continue;

      cfgEntry += opt->m_longFlag + "=" + opt->m_defaultValue + "\n";
   }

   return cfgEntry;
}


/* setup tools. */
void Valkyrie::initToolObjects()
{ 
   /* connect all tool::fatal() signals to this::quit() */
   ToolObjList toolObjList = valgrind()->toolObjList();
   for ( ToolObject* tool=toolObjList.first(); tool; tool=toolObjList.next() ) {
      connect( tool, SIGNAL( fatal() ), this, SLOT( quit() ) );
   }
}


/* slot: called by tools, if they have a fatal() accident */
void Valkyrie::quit()
{ exit(0); }


/* Returns a '\n' separated list of current relevant flags.
   Called by MainWindow::showFlagsWidget()
   - which is triggered by optionsWin::flagsChanged signal
*/
QString Valkyrie::getDisplayFlags()
{
   QString flagsStr = m_flags.join("\n");
   if ( !vkConfig->rdEntry("binary-flags", "valkyrie").isEmpty() )
      flagsStr.replace( flagsStr.findRev('\n'), 1, ' ');
   return flagsStr;
}


/* called from Valkyrie::updateVgFlags() 
   see if valkyrie was told what to do:
   - on the cmd-line
   - via a tool
   - via the gui options page */
QStringList Valkyrie::modifiedVgFlags()
{
   Option*     opt    = findOption( BINARY );
   QString     cfgVal = vkConfig->rdEntry( opt->m_longFlag, name() );

   /* cfgVal may not be valid here (may be empty, or old),
      but checked in MainWindow::run() anyway. */

   /* only add binary & bin_flags if binary present */
   QStringList modFlags;
   if ( cfgVal != opt->m_defaultValue ) {
      modFlags << cfgVal;
      /* see if there were any flags given for the binary */
      opt = findOption( BIN_FLAGS );
      cfgVal = vkConfig->rdEntry( opt->m_longFlag, name() );
      modFlags += QStringList::split(" ", cfgVal);
   }
   return modFlags;
}


/* Update flags for current tool
   Called by MainWindow::updateVgFlags()
   - which is triggered by optionsWin::flagsChanged signal
*/
void Valkyrie::updateVgFlags( int tId )
{
   ToolObject* tool = valgrind()->toolObj( tId );

   QString vg_exec = vkConfig->rdEntry( "vg-exec","valkyrie");
   if (vg_exec.isEmpty())
      vg_exec = "valgrind";    /* hope it's in $PATH */

   /* modifiedVgFlags() functions return non-default flags */
   m_flags.clear();
   m_flags << vg_exec;                                  // path/to/valgrind
   m_flags << "--tool=" + tool->name();                 // tool
   m_flags += valgrind()->modifiedVgFlags( tool );      // valgrind opts
   m_flags += tool->modifiedVgFlags();                  // tool opts
   m_flags += this->modifiedVgFlags();                  // valkyrie opts
}


/* called from MainWin when user clicks stopButton */
void Valkyrie::stopTool( int tId )
{
   ToolObject* tool = valgrind()->toolObj( tId );
   vk_assert( tool != 0 );

   tool->stop();
   vk_assert( !tool->isRunning() );
}


/* Run the tool with given runState */
bool Valkyrie::runTool( int tId, VkRunState::State runState )
{
   ToolObject* activeTool = valgrind()->toolObj( tId );
   vk_assert( activeTool != 0 );

   return activeTool->start( runState, m_flags );
}


bool Valkyrie::queryToolDone( int tId )
{
   ToolObject* tool = valgrind()->toolObj( tId );
   vk_assert( tool != 0 );

   return tool->queryDone();
}


/* Run the tool with given runState */
VkRunState::State Valkyrie::startRunState()
{ return m_startRunState; }



/* Return VkObject with given objId */
VkObject* Valkyrie::vkObject( int objId )
{
   switch (objId) {
   case VkObject::ID_VALKYRIE: return this;
   case VkObject::ID_VALGRIND: return valgrind();
   default:                    return valgrind()->toolObj( objId );
   }
}


/* Return list of all VkObjects */
VkObjectList Valkyrie::vkObjList()
{
   VkObjectList vkObjList; // don't delete contents!

   vkObjList.append(this);
   vkObjList.append(m_valgrind);

   ToolObjList tools = valgrind()->toolObjList();
   for ( ToolObject* tool = tools.first(); tool; tool = tools.next() )
      vkObjList.append( tool );

   return vkObjList;
}

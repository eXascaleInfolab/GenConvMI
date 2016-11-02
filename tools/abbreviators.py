#!/usr/bin/python
# vim: set fileencoding=utf-8 :


from __future__ import print_function

import os
import os.path
import sys

from abc import ABCMeta, abstractmethod

# class RequiresStateWatcher( object ): {{{
class RequiresStateWatcher( object ):
    __metaclass__ = ABCMeta

    def setStateWatcher( self, state_watcher ):
        self._state_watcher = state_watcher
# }}}

# class StateWatcher( object ): {{{
class StateWatcher( object ):
    
    def __init__(self):
        self._state = 'first-lines'
        
    def feedLine( self, line ):
        if self._state == 'first-lines':
            if line == 'Flat profile:':
                self._state = 'flat-profile-prologue'
                self._seen_lines = 1
        elif self._state == 'flat-profile-prologue':
            if self._seen_lines < 5:
                self._seen_lines += 1
            else:
                self._state = 'flat-profile'
                self.name_field_start = 54
        elif self._state == 'flat-profile':
            if line in ('', '\n', ' \n\r'):
                self._state = 'end-flat-profile'
        elif self._state == 'end-flat-profile':
            self._state = 'call-profile-prologue'
        elif self._state == 'call-profile-prologue':
            if line.find( '<spontaneous>' ) != -1:
                self._state = 'call-profile-inv-point0'
                self.name_field_start = 49
        elif self._state in( 'call-profile-inv-point' , 'call-profile-inv-point0' ) \
                and len( line ) > 0 and line[0] == '[' :
            self._state = 'call-profile-target'
            self.name_field_start = 45
        elif self._state == 'call-profile-inv-point0':
            self._state = 'call-profile-inv-point'
            self.name_field_start = 49
        elif self._state == 'call-profile-target':
            if len( line ) > 0 and line[0] == '-':
                self._state = 'call-profile-separator'
            else:
                self._state = 'call-profile-callees'
                self.name_field_start = 49
        elif self._state == 'call-profile-callees':
            if len( line ) > 0 and line[0] == '-':
                self._state = 'call-profile-separator'
        elif self._state == 'call-profile-separator':
            if len( line ) > 0:
                self._state = 'call-profile-inv-point0'
            else:
                self._state = 'explanations'

    def containNames( self ):
        if self._state in ( 
            'flat-profile',
            'call-profile-inv-point',
            'call-profile-inv-point0',
            'call-profile-target',
            'call-profile-callees'
            ):
            return True

        return False

    @property
    def state( self ):
        return self._state
# }}}

# class Filter( object ): {{{
class Filter( object ):
    
    def __call__( self, line ):
        return None
# }}}

# class NameFilter( Filter, RequiresStateWatcher ): {{{
class NameFilter( Filter, RequiresStateWatcher ):

    def __init__(self ):
        #self._state_watcher = state_watcher
        pass

    def getNameFromLine( self, line ):
        return line[ self._state_watcher.name_field_start: ]

    def __call__(self, line):
        if self._state_watcher.containNames():
            return self.effect( line )
        else:
            return None

    def effect( self, line ):
        return line
# }}}

# class ReconFilter( NameFilter ): {{{
class ReconFilter( NameFilter ):
    
    def __init__(self, state_watcher, phrase ):
        NameFilter.__init__(self, state_watcher )
        self._phrase = phrase

    def effect(self, line ):
        whole_name = self.getNameFromLine( line )
        if whole_name.find( self._phrase ) != -1:
            result = line.replace(whole_name, self._phrase ) 
            return result
        else:
            return None
# }}}

# class Continue( object ): {{{
class Continue( object ):
    def __init__(self, line):
        self.line = line
# }}}

# class SubstFilter( NameFilter ): {{{
class SubstFilter( NameFilter ):
    def __init__(self, target_phrase, new_phrase ):
        NameFilter.__init__(self )
        self._target_phrase = target_phrase
        self._new_phrase = new_phrase
            
    def effect( self, line ):
        whole_name = self.getNameFromLine( line )
        #print( whole_name, file = sys.stderr )
        new_whole_name = whole_name.replace( 
            self._target_phrase,
            self._new_phrase )
        had_effect = new_whole_name != whole_name
        wn_pos = line.find( whole_name )
        wn_len = len( whole_name )
        if had_effect:
            ln = line[:wn_pos] + u'⬩' + new_whole_name + line[wn_pos+wn_len:] 
        else:
            ln = line
        return Continue( ln )
# }}}

class SectionsFilter( Filter, RequiresStateWatcher ):
    def __init__( self ):
        #self._state_watcher = state_watcher
        self._call_n = 0

    def __call__(self, line ):
        if line.find( 'Flat profile:' ) != -1 :
            return 'Flat profile: {''{{ ' 
        else:
            if self._state_watcher.state == 'end-flat-profile':
                return '}''}}'
            elif self._state_watcher.state == 'call-profile-inv-point0':
                self._call_n += 1
                return Continue( '{{''{' + "[{0:04}]".format(self._call_n) + line[9:] )
            elif self._state_watcher.state == 'call-profile-separator':
                return '}}''}' + line[3:]
        
        return None

_filter_list = [
   SectionsFilter(),
   SubstFilter("overmap::detail::optimizer2<boost::adjacency_list<boost:" + 
               ":vecS, boost::vecS, boost::undirectedS, overmap::undirected_vertex_info,"+
               " overmap::undirected_edge_info, boost::no_property, boost::listS> >",
               "optimizer2" ),
   SubstFilter("overmap::detail::crossings<boost::adjacency_list<boost::vecS,"
                " boost::vecS, boost::undirectedS, overmap::undirected_vertex_info,"
                " overmap::undirected_edge_info, boost::no_property, boost::listS>,"
                " overmap::detail::probability_datum>",
                "crossings"
                ),
    SubstFilter("unsigned long", "size_t"),
    SubstFilter("boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, overmap::undirected_vertex_info, overmap::undirected_edge_info, boost::no_property, boost::listS>","graph_t"),
    SubstFilter("overmap::condprobcalc::vertex_iteration<graph_t, boost::bundle_property_map<graph_t, boost::detail::edge_desc_impl<boost::undirected_tag, size_t>, overmap::undirected_edge_info, double>, boost::bundle_property_map<graph_t, size_t, overmap::undirected_vertex_info, double> >", 
        "vertex_iteration"),
    SubstFilter("overmap::map_eq_calculator<overmap::graph_messenger<graph_t, overmap::undirected_vertex_info, overmap::undirected_edge_info> >", "map_eq_calculator" ),
]

def main():
    state_watcher = StateWatcher() 
    for fl in _filter_list:
        if isinstance( fl, RequiresStateWatcher ):
            fl.setStateWatcher( state_watcher )
    for line in sys.stdin:
        ln = line.replace('\n', '')
        ln = ln.replace('\r', '')
        state_watcher.feedLine( ln )    
        for fl in _filter_list:
            result = fl( ln )
            if result != None:
                if isinstance( result, Continue ):
                    ln = result.line
                else:
                    ln = result
                    break
        print( ln.encode('utf-8'), file = sys.stdout )
    

if __name__ == "__main__":
    main()


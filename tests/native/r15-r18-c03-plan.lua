-- Actual trusted check planner regression; no native or policy shim.
local f=assert(io.open('src/stdlib/planar/checks.lua'));local source=f:read('*a');f:close()
local checks=assert(load("local dispatch_internal={locate=function()return {spec={work={{id='a',paths={'a/'}},{id='b',paths={'b/'}},{id='c',paths={'c/'}},{id='z',paths={'z/'}},{id='q',paths={'q/'}},{id='x',paths={'x/'}}}},bundle={registry={}}}end}\n"..source..'\nreturn checks'))()
local W={root='r',base={label='b'},ownership={frozen={},excluded={}},units={{id='a',paths={'a/'}},{id='b',paths={'b/'}},{id='c',paths={'c/'}}},dependencies={{id='internal',consumer='b',producer='a',phase='complete',source='facet'},{id='external',consumer='c',producer='z',phase='complete',source='facet'},{id='start',consumer='b',producer='q',phase='start',source='facet'},{id='nonvector',consumer='a',producer='x',phase='complete',source='contract'}},checks={{id='integration',toolchain='sh',inputs={'own_output','output_vector'}}},toolchains={{id='sh',command={'/bin/sh'},environment={}}},effective_checks={{work='b',checks={'integration'}},{work='c',checks={'integration'}}}}
checks.hooks.current_work=function()return {work=W}end
local S={revision={interpretation={}},active_job='j',roles={{job='j',role='worker',generation={id='1',structure={kind='worker',work={'a','b','c'}}},run={id='1'}}}}
local P=assert(checks.plan(S,'worker'))
assert(table.concat(P.checks[1].vector,',')=='a,b','grouped producer retained; unrelated producer excluded')
assert(table.concat(P.checks[2].vector,',')=='c,z','external producer retained')
assert(#P.producers==1 and P.producers[1].work=='z','capture only required external vector producers')
print('SUMMARY tests=3 failures=0 skips=0')

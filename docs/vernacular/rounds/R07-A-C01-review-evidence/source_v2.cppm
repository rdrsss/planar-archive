export module planar.kernel.runtime_source;
import std;
export namespace planar::kernel::embedded {
std::string_view source() { return R"LUA(return {
 repeated = function() while true do pcall(function() while true do end end) end end,
 caught = function() local ok = pcall(function() while true do end end); return ok end,
 handler = function() xpcall(function() while true do end end, function() while true do end end); return true end,
})LUA"; }
}

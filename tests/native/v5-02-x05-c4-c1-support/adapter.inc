// Verification-only adapter; never a production service or capability.
int project_probe(lua_State* l) {
  bool failed = true;
  if (lua_gettop(l) == 1 && lua_type(l, 1) == LUA_TSTRING) {
    std::size_t size = 0;
    auto        id   = lua_tolstring(l, 1, &size);
    try {
      c4_stage(10);
      auto result = commands(l).extension_history.owner->private_project({id, size});
      c4_stage(11);
      if (result) {
        commands(l).response = std::move(result->result);
        failed               = false;
      } else if (!commands(l).shape_failure) {
        commands(l).shape_failure = std::move(result.error());
      }
    } catch (const std::bad_alloc&) {
      context(l).native_memory_failed = true;
    } catch (const std::length_error&) {
      context(l).native_memory_failed = true;
    } catch (...) {
    }
  }
  if (failed) {
    commands(l).extension_history.refused = true;
    return luaL_error(l, "private_project_probe");
  }
  lua_settop(l, 0);
  push(l, commands(l).response.get());
  return 1;
}

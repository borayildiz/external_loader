# Corner cases

## ICACHE / DCACHE vs Verify

`Verify` reads the programmed image via CPU loads from the memory-mapped window at `0xA0000000`. Erase/program update the NOR through **indirect** HSPI commands and do not update ICACHE/DCACHE.

If earlier firmware left the caches enabled with lines for `0xA0…`, a cache **hit** during verify returns the **old** value instead of the new NOR contents. `IS25_EnableMemoryMapped()` does not flush the caches.

**Mitigation:** `Loader_DisableCaches()` disables ICACHE (auto-invalidate) and invalidates then disables DCACHE1. Called from `Init`, and again before mmap `Read` / `Verify` / `CheckSum`.

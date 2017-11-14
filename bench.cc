#include "redismodule.h"

#include <iostream>
#include <string>
#include <vector>

static constexpr char kModuleName[] = "RedisModuleBench";

// "FillKeys N": sets to a trivial hash for keys {1, ..., N}.
int FillKeys_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                          int argc) {

  if (argc != 2) {
    return RedisModule_WrongArity(ctx);
  }
  long long N = 0;
  RedisModule_StringToLongLong(argv[1], &N);

  const int kNumBytes = 128;
  std::vector<char> buffer;
  buffer.resize(kNumBytes);
  RedisModuleString *kVal =
      RedisModule_CreateString(ctx, buffer.data(), kNumBytes);

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    RedisModuleString *i_str = RedisModule_CreateStringFromLongLong(ctx, i);
    RedisModuleKey *key = static_cast<RedisModuleKey *>(
        RedisModule_OpenKey(ctx, i_str, REDISMODULE_READ | REDISMODULE_WRITE));
    // RedisModule_HashSet(key, REDISMODULE_HASH_CFIELDS, "val0", kVal, NULL);
    RedisModule_HashSet(key, REDISMODULE_HASH_CFIELDS, "val0", kVal, "val1",
                        kVal, "val2", kVal, "val3", kVal, NULL);
    RedisModule_CloseKey(key);
    RedisModule_FreeString(ctx, i_str);
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Fill(" << N << ") took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << " millis" << std::endl;

  // RedisModuleCtx *rctx = RedisModule_GetThreadSafeContext(NULL);
  // RedisModule_ThreadSafeContextLock(rctx);
  // std::cout << "rctx addr " << rctx << "\n";
  // RedisModule_Call(rctx, "KEYS", "*");
  // RedisModule_ThreadSafeContextUnlock(rctx);
  // RedisModule_FreeThreadSafeContext(rctx);

  RedisModule_FreeString(ctx, kVal);
  RedisModule_ReplyWithSimpleString(ctx, "OK");

  return REDISMODULE_OK;
}

// "DelKeys N": delete hash keys {1, ..., N}.
// Measured block:
//    for i in range(0, N):
//        k = RedisModule_OpenKey(...pre-filled key contents...)
//        RedisModule_DeleteKey(k)
//        RedisModule_CloseKey(k)
int DelKeys_RedisCommand(RedisModuleCtx *ctx, RedisModuleString **argv,
                         int argc) {

  if (argc != 2) {
    return RedisModule_WrongArity(ctx);
  }
  long long N = 0;
  RedisModule_StringToLongLong(argv[1], &N);
  RedisModuleString *kVal = RedisModule_CreateString(ctx, "0", strlen("0"));

  std::vector<RedisModuleString *> strings;
  strings.reserve(N);
  for (int i = 0; i < N; ++i) {
    strings.push_back(RedisModule_CreateStringFromLongLong(ctx, i));
  }

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();
  for (int i = 0; i < N; ++i) {
    RedisModuleKey *key = static_cast<RedisModuleKey *>(RedisModule_OpenKey(
        ctx, strings[i], REDISMODULE_READ | REDISMODULE_WRITE));
    RedisModule_DeleteKey(key);
    RedisModule_CloseKey(key);
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Del(" << N << ") took "
            << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                     begin)
                   .count()
            << " millis" << std::endl;

  // Cleanup.
  for (int i = 0; i < N; ++i) {
    RedisModule_FreeString(ctx, strings[i]);
  }
  RedisModule_FreeString(ctx, kVal);
  RedisModule_ReplyWithSimpleString(ctx, "OK");

  return REDISMODULE_OK;
}

extern "C" {
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv,
                       int argc) {
  REDISMODULE_NOT_USED(argc);
  REDISMODULE_NOT_USED(argv);
  if (RedisModule_Init(ctx, kModuleName, 1, REDISMODULE_APIVER_1) ==
      REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  // Registers module commands.
  if (RedisModule_CreateCommand(ctx, "fillkeys", FillKeys_RedisCommand, "write",
                                1, 1, 1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  if (RedisModule_CreateCommand(ctx, "delkeys", DelKeys_RedisCommand, "write",
                                1, 1, 1) == REDISMODULE_ERR) {
    return REDISMODULE_ERR;
  }
  return REDISMODULE_OK;
}
}

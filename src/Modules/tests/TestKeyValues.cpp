#include "TestKeyValues.h"
#include "SDK/KeyValues.h"

static TestKeyValues g_test_keyvalues;

void TestKeyValues::StartListening()
{
    {
        KeyValues::AutoDelete kv = KeyValues::AutoDelete("test");
        kv->SetInt("outputwav", 1);
        kv->SetString("filename", "/example/file/name");
        kv->SetFloat("framerate", 600);

        printf("KeyValues test\n");
        printf("outputwav -> %d\n", kv->GetInt("outputwav"));
        printf("filename -> %s\n", kv->GetString("filename", "(invalid)"));
        printf("framerate -> %f\n", kv->GetFloat("framerate", -1.f));
    }
    printf("Deleted KeyValues without crashing\n");
}
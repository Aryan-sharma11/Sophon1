#include <Ntifs.h>
#include <ntddk.h>
#include <wdf.h>

UNICODE_STRING DEVICE_NAME = RTL_CONSTANT_STRING(L"\\Device\\BlockProcess");
UNICODE_STRING SYM_LINK = RTL_CONSTANT_STRING(L"\\??\\BlockProcess");
void CreateProcessNotifyRoutine(PEPROCESS process, HANDLE pid, PPS_CREATE_NOTIFY_INFO createInfo) {
    UNREFERENCED_PARAMETER(process);
    UNREFERENCED_PARAMETER(pid);
    DbgPrint("Create ProcessNotifyRoutine called\n");


    // Never forget this if check because if you don't, you'll end up crashing your Windows system ;P
    if (createInfo != NULL) {
        // Compare the command line of the launched process to the notepad string
        DbgPrint("Process Name (%wZ).\n", createInfo->CommandLine);
        if (wcsstr(createInfo->CommandLine->Buffer, L"Notepad") != NULL) {
            DbgPrint("Process (%wZ) denied.\n", createInfo->CommandLine);
            // Process allowed
            createInfo->CreationStatus = STATUS_ACCESS_DENIED;
        }
    }
}


void UnloadMyDumbEDR(_In_ PDRIVER_OBJECT DriverObject) {
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "Unloading routine called\n");
    // Unset the callback
    PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyRoutine, TRUE);
    // Delete the driver device 
    IoDeleteDevice(DriverObject->DeviceObject);
    // Delete the symbolic link
    IoDeleteSymbolicLink(&SYM_LINK);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {

    UNREFERENCED_PARAMETER(RegistryPath);
    DbgPrint("Updsated driver entry\n");

    DbgPrint("Initializing the driver\n");

    NTSTATUS status;

    // Setting the unload routine to execute
    DriverObject->DriverUnload = UnloadMyDumbEDR;

    // Initializing a device object and creating it
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING deviceName = DEVICE_NAME;
    UNICODE_STRING symlinkName = SYM_LINK;
    status = IoCreateDevice(
        DriverObject,		   // our driver object,
        0,					   // no need for extra bytes,
        &deviceName,           // the device name,
        FILE_DEVICE_UNKNOWN,   // device type,
        0,					   // characteristics flags,
        FALSE,				   // not exclusive,
        &DeviceObject		   // the resulting pointer
    );
    if (!NT_SUCCESS(status)) {
        DbgPrint("MyDumbEDR: Device creation failed: 0x%X\n", status);
        return status;
    }

    status = IoCreateSymbolicLink(&symlinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Symlink creation failed\n");
        IoDeleteDevice(DeviceObject);
        return status;
    }

    PsSetCreateProcessNotifyRoutineEx(CreateProcessNotifyRoutine, FALSE);

    return STATUS_SUCCESS;
}
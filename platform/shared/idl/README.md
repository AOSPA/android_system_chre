This folder contains FlatBuffers schema used in the communications protocol
between the reference CHRE implementation and the host (applications processor).

Use the included update.sh script to generate the header files used in CHRE,
which requires that the FlatBuffers compiler `flatc` be available in $PATH.

FlatBuffers compiler version 1.12.0 must be used since some modifications are
made to the version of flatbuffers header used by the generated code.

For more information on FlatBuffers, see https://github.com/google/flatbuffers/

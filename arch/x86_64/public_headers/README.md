# Arch specific-headers

A way to expose typedefs, macros and struct definitions to rest of the kernel, part of the API

Keep in mind that everything in here is visible to rest of the kernel

## RULES

- Don't put things that can go to stable API (/include/arch) here
- Don't add things that don't need be here

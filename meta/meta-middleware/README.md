# meta-middleware

Yocto/OpenEmbedded layer providing communication middleware for automotive ECU applications.

## Description

This layer provides the core communication infrastructure used across all ECU2 applications:
- **vsomeip**: SOME/IP implementation for service-oriented communication
- **CommonAPI-Core**: Language-independent middleware API
- **CommonAPI-SomeIP**: SOME/IP binding for CommonAPI

## Layer Contents

- `recipes-comm/vsomeip/` - SOME/IP implementation (v3.5.8)
- `recipes-comm/commonapi-core/` - CommonAPI core runtime (v3.2.4)
- `recipes-comm/commonapi-someip/` - CommonAPI SOME/IP runtime (v3.2.4)
- `recipes-comm/vsomeip-config/` - vsomeip configuration files

## Dependencies

- meta (OE-Core)
- Boost libraries (provided by meta-openembedded)

## Usage

Add this layer to your `bblayers.conf`:

```
BBLAYERS += "/path/to/DES_Head-Unit/meta/meta-middleware"
```

## Compatibility

- Yocto: kirkstone, langdale, mickledore, nanbield, scarthgap
- Target: ARM64 (Raspberry Pi 4)

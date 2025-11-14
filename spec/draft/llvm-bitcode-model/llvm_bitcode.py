# This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild
# type: ignore

import kaitaistruct
from kaitaistruct import KaitaiStruct, KaitaiStream, BytesIO


if getattr(kaitaistruct, 'API_VERSION', (0, 9)) < (0, 11):
    raise Exception("Incompatible Kaitai Struct Python API: 0.11 or later is required, but you have %s" % (kaitaistruct.__version__))

class LlvmBitcode(KaitaiStruct):
    """LLVM Bitcode File Format
    ========================
    
    This Kaitai Struct specification models the LLVM Bitcode file format at a container level, including support for:
    - The Bitcode Wrapper format (commonly seen in `.bc` files) and
    - A generic container for the LLVM IR bitstream payload.
    
    This specification focuses on:
    - Validating and extracting the Bitcode Wrapper header (magic, version, bitcode offset and size),
    - Exposing the bitstream payload as a substream for downstream parsers or tools,
    - Providing exhaustive documentation (as Markdown) of the LLVM Bitstream and LLVM IR encoding to accompany the structure.
    
    Note on bit-level parsing and VBRs:
    - LLVM bitstreams use bit-level encodings, including variable bit rate integers (VBR) and dynamic bit-width fields.
    - While the Kaitai Struct language supports bit-level reading, general-purpose VBR decoding and dynamically sized bit fields require procedural logic that goes beyond declarative YAML. For this reason, this spec validates and extracts the payload cleanly and documents the lower-level bit encodings thoroughly, so that specialized consumers can decode the payload if needed.
    
    Reference
    ---------
    - LLVM Bitcode File Format (official documentation): https://llvm.org/docs/BitCodeFormat.html
    
    The content below copies and distributes the documentation into the appropriate sections of this specification, using Markdown (`md`) formatting for clarity.
    
    Abstract
    --------
    This document describes the LLVM bitstream file format and the encoding of the LLVM IR into it.
    
    Overview
    --------
    What is commonly known as the LLVM bitcode file format (also, sometimes anachronistically known as bytecode) is actually two things: a bitstream container format and an encoding of LLVM IR into the container format.
    
    The bitstream format is an abstract encoding of structured data, very similar to XML in some ways. Like XML, bitstream files contain tags, and nested structures, and you can parse the file without having to understand the tags. Unlike XML, the bitstream format is a binary encoding, and unlike XML it provides a mechanism for the file to self-describe “abbreviations”, which are effectively size optimizations for the content.
    
    LLVM IR files may be optionally embedded into a wrapper structure, or in a native object file. Both of these mechanisms make it easy to embed extra data along with LLVM IR files.
    
    This document first describes the LLVM bitstream format, describes the wrapper format, then describes the record structure used by LLVM IR files.
    
    Bitstream Format
    ----------------
    The bitstream format is literally a stream of bits, with a very simple structure. This structure consists of the following concepts:
    - A “magic number” that identifies the contents of the stream.
    - Encoding primitives like variable bit-rate integers.
    - Blocks, which define nested content.
    - Data Records, which describe entities within the file.
    - Abbreviations, which specify compression optimizations for the file.
    
    Note that the `llvm-bcanalyzer` tool can be used to dump and inspect arbitrary bitstreams, which is very useful for understanding the encoding.
    
    Magic Numbers
    -------------
    The first four bytes of a bitstream are used as an application-specific magic number. Generic bitcode tools may look at the first four bytes to determine whether the stream is a known stream type. However, these tools should not determine whether a bitstream is valid based on its magic number alone. New application-specific bitstream formats are being developed all the time; tools should not reject them just because they have a hitherto unseen magic number.
    
    Primitives
    ----------
    A bitstream literally consists of a stream of bits, which are read in order starting with the least significant bit of each byte. The stream is made up of a number of primitive values that encode a stream of unsigned integer values. These integers are encoded in two ways: either as Fixed Width Integers or as Variable Width Integers.
    
    Fixed Width Integers
    --------------------
    Fixed-width integer values have their low bits emitted directly to the file. For example, a 3-bit integer value encodes 1 as 001. Fixed-width integers are used when there are a well-known number of options for a field. For example, boolean values are usually encoded with a 1-bit-wide integer.
    
    Variable Width Integers (VBR)
    -----------------------------
    Variable-width integer (VBR) values encode values of arbitrary size, optimizing for the case where the values are small. Given a 4-bit VBR field, any 3-bit value (0 through 7) is encoded directly, with the high bit set to zero. Values larger than N-1 bits emit their bits in a series of N-1 bit chunks, where all but the last set the high bit.
    
    For example, the value 30 (0x1E) is encoded as 62 (0b0011’1110) when emitted as a vbr4 value. The first set of four bits starting from the least significant indicates the value 6 (110) with a continuation piece (indicated by a high bit of 1). The next set of four bits indicates a value of 24 (011 << 3) with no continuation. The sum (6+24) yields the value 30.
    
    6-bit characters
    ----------------
    6-bit characters encode common characters into a fixed 6-bit field. They represent the following characters with the following 6-bit values:
    - 'a' .. 'z' — 0 .. 25
    - 'A' .. 'Z' — 26 .. 51
    - '0' .. '9' — 52 .. 61
    - '.' — 62
    - '_' — 63
    
    This encoding is only suitable for encoding characters and strings that consist only of the above characters. It is completely incapable of encoding characters not in the set.
    
    Word Alignment
    --------------
    Occasionally, it is useful to emit zero bits until the bitstream is a multiple of 32 bits. This ensures that the bit position in the stream can be represented as a multiple of 32-bit words.
    
    Abbreviation IDs
    ----------------
    A bitstream is a sequential series of Blocks and Data Records. Both of these start with an abbreviation ID encoded as a fixed-bitwidth field. The width is specified by the current block. The value of the abbreviation ID specifies either a builtin ID (which have special meanings) or one of the abbreviation IDs defined for the current block by the stream itself.
    
    The set of builtin abbrev IDs includes (non-exhaustive list):
    - 0 — END_BLOCK
    - 1 — ENTER_SUBBLOCK
    - 2 — DEFINE_ABBREV
    - 3 — UNABBREV_RECORD
    
    Blocks
    ------
    Blocks define nested content with their own abbreviation widths and local abbreviation tables:
    - ENTER_SUBBLOCK Encoding
    - END_BLOCK Encoding
    
    Data Records
    ------------
    Data records describe entities within the file:
    - UNABBREV_RECORD Encoding
    - Abbreviated Record Encoding
    
    Abbreviations
    -------------
    Blocks can define abbreviations to compress record encodings:
    - DEFINE_ABBREV Encoding
    
    Standard Blocks
    ---------------
    - #0 — BLOCKINFO Block
    
    Bitcode Wrapper Format
    ----------------------
    LLVM IR files may be embedded in a simple wrapper that starts with the four-byte magic number `'BC' 0xC0 0xDE` (little-endian composite value 0xDEC04342). The wrapper typically encodes:
    - A 32-bit version (commonly 1),
    - A 32-bit offset to the embedded bitstream (commonly 16),
    - A 32-bit size of the embedded bitstream,
    - A 32-bit reserved field.
    
    This Kaitai spec validates the wrapper fields and exposes the bitstream payload as a substream.
    
    Native Object File Wrapper Format
    ---------------------------------
    LLVM IR may also be embedded within native object file formats (e.g., Mach-O, COFF) using dedicated sections. This spec focuses on the standalone Bitcode Wrapper.
    
    LLVM IR Encoding
    ----------------
    The LLVM IR is encoded into the bitstream using a series of blocks. Important concepts include:
    - Basics
      - LLVM IR Magic Number
      - Signed VBRs
      - LLVM IR Blocks
    - MODULE_BLOCK Contents
      - MODULE_CODE_VERSION Record
      - MODULE_CODE_TRIPLE Record
      - MODULE_CODE_DATALAYOUT Record
      - MODULE_CODE_ASM Record
      - MODULE_CODE_SECTIONNAME Record
      - MODULE_CODE_DEPLIB Record
      - MODULE_CODE_GLOBALVAR Record
      - MODULE_CODE_FUNCTION Record
      - MODULE_CODE_ALIAS Record
      - MODULE_CODE_GCNAME Record
    - PARAMATTR_BLOCK Contents
      - PARAMATTR_CODE_ENTRY Record
      - PARAMATTR_CODE_ENTRY_OLD Record
    - PARAMATTR_GROUP_BLOCK Contents
      - PARAMATTR_GRP_CODE_ENTRY Record
    - TYPE_BLOCK Contents
      - TYPE_CODE_NUMENTRY Record
      - TYPE_CODE_VOID Record
      - TYPE_CODE_HALF Record
      - TYPE_CODE_BFLOAT Record
      - TYPE_CODE_FLOAT Record
      - TYPE_CODE_DOUBLE Record
      - TYPE_CODE_LABEL Record
      - TYPE_CODE_OPAQUE Record
      - TYPE_CODE_INTEGER Record
      - TYPE_CODE_POINTER Record
      - TYPE_CODE_FUNCTION_OLD Record
      - TYPE_CODE_ARRAY Record
      - TYPE_CODE_VECTOR Record
      - TYPE_CODE_X86_FP80 Record
      - TYPE_CODE_FP128 Record
      - TYPE_CODE_PPC_FP128 Record
      - TYPE_CODE_METADATA Record
      - TYPE_CODE_X86_MMX Record
      - TYPE_CODE_STRUCT_ANON Record
      - TYPE_CODE_STRUCT_NAME Record
      - TYPE_CODE_STRUCT_NAMED Record
      - TYPE_CODE_FUNCTION Record
      - TYPE_CODE_X86_AMX Record
      - TYPE_CODE_TARGET_TYPE Record
    - CONSTANTS_BLOCK Contents
    - FUNCTION_BLOCK Contents
    - VALUE_SYMTAB_BLOCK Contents
    - METADATA_BLOCK Contents
    - METADATA_ATTACHMENT Contents
    - STRTAB_BLOCK Contents
    
    STRTAB_BLOCK
    ------------
    The `STRTAB` block (id 23) contains a single record (`STRTAB_BLOB`, id 1) with a single blob operand containing the bitcode file’s string table.
    
    Strings in the string table are not null terminated. A record’s strtab offset and strtab size operands specify the byte offset and size of a string within the string table.
    
    The string table is used by all preceding blocks in the bitcode file that are not succeeded by another intervening `STRTAB` block. Normally a bitcode file will have a single string table, but it may have more than one if it was created by binary concatenation of multiple bitcode files.
    
    Copyright
    ---------
    © 2003–2025, LLVM Project. See the upstream documentation for any updates. This spec mirrors the upstream documentation organization for ease of cross-reference.
    
    Source: https://llvm.org/docs/BitCodeFormat.html
    """
    def __init__(self, _io, _parent=None, _root=None):
        super(LlvmBitcode, self).__init__(_io)
        self._parent = _parent
        self._root = _root or self
        self._read()

    def _read(self):
        self.magic = self._io.read_u4le()
        _on = self.magic
        if _on == 3737142082:
            pass
            self.contents = LlvmBitcode.WrapperFile(self._io, self, self._root)
        else:
            pass
            self.contents = LlvmBitcode.RawBitcodeContainer(self._io, self, self._root)


    def _fetch_instances(self):
        pass
        _on = self.magic
        if _on == 3737142082:
            pass
            self.contents._fetch_instances()
        else:
            pass
            self.contents._fetch_instances()

    class RawBitcodeContainer(KaitaiStruct):
        """Raw LLVM IR Bitstream container without the Bitcode Wrapper.
        
        This structure exposes the entire file as a bitstream payload. Consumers can pass this substream to bitstream decoders that understand LLVM’s block, record, and abbreviation encodings.
        
        .. seealso::
           Source - https://llvm.org/docs/BitCodeFormat.html
        """
        def __init__(self, _io, _parent=None, _root=None):
            super(LlvmBitcode.RawBitcodeContainer, self).__init__(_io)
            self._parent = _parent
            self._root = _root
            self._read()

        def _read(self):
            self.payload = self._io.read_bytes(self._io.size() - 4)


        def _fetch_instances(self):
            pass
            _ = self.full_stream
            if hasattr(self, '_m_full_stream'):
                pass


        @property
        def full_stream(self):
            """Full raw bitstream including the initial 4-byte magic."""
            if hasattr(self, '_m_full_stream'):
                return self._m_full_stream

            _pos = self._io.pos()
            self._io.seek(0)
            self._m_full_stream = self._io.read_bytes(self._io.size())
            self._io.seek(_pos)
            return getattr(self, '_m_full_stream', None)


    class WrapperFile(KaitaiStruct):
        """LLVM Bitcode Wrapper file.
        
        Layout (all 32-bit little-endian fields):
        - magic:   0xDEC04342  (`'B' 'C' 0xC0 0xDE`)
        - version: commonly 1
        - offset:  byte offset to the embedded bitstream (commonly 16)
        - size:    size in bytes of the embedded bitstream
        - reserved: 32-bit reserved value (often 0)
        
        The bitstream payload is exposed via the `bitstream` instance as a substream of length `size` starting at `offset` from the beginning of the file.
        
        .. seealso::
           Source - https://llvm.org/docs/BitCodeFormat.html
        """
        def __init__(self, _io, _parent=None, _root=None):
            super(LlvmBitcode.WrapperFile, self).__init__(_io)
            self._parent = _parent
            self._root = _root
            self._read()

        def _read(self):
            self.version = self._io.read_u4le()
            if not self.version == 1:
                raise kaitaistruct.ValidationNotEqualError(1, self.version, self._io, u"/types/wrapper_file/seq/0")
            self.offset = self._io.read_u4le()
            self.size = self._io.read_u4le()
            self.reserved = self._io.read_u4le()


        def _fetch_instances(self):
            pass
            _ = self.bitstream
            if hasattr(self, '_m_bitstream'):
                pass


        @property
        def bitstream(self):
            """Substream of the embedded LLVM IR bitstream payload."""
            if hasattr(self, '_m_bitstream'):
                return self._m_bitstream

            _pos = self._io.pos()
            self._io.seek(self.offset)
            self._m_bitstream = self._io.read_bytes(self.size)
            self._io.seek(_pos)
            return getattr(self, '_m_bitstream', None)

        @property
        def is_offset_aligned_4(self):
            if hasattr(self, '_m_is_offset_aligned_4'):
                return self._m_is_offset_aligned_4

            self._m_is_offset_aligned_4 = self.offset % 4 == 0
            return getattr(self, '_m_is_offset_aligned_4', None)

        @property
        def is_size_valid(self):
            if hasattr(self, '_m_is_size_valid'):
                return self._m_is_size_valid

            self._m_is_size_valid =  ((self.size > 0) and (self.offset + self.size <= self._io.size())) 
            return getattr(self, '_m_is_size_valid', None)

        @property
        def is_version_supported(self):
            if hasattr(self, '_m_is_version_supported'):
                return self._m_is_version_supported

            self._m_is_version_supported = self.version == 1
            return getattr(self, '_m_is_version_supported', None)




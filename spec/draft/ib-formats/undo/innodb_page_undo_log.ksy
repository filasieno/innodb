# Undo log page containing undo records for MVCC and transaction rollback
# Reference: MySQL 8.0 Source (storage/innobase/include/trx0undo.h, trx0rec.h)
meta:
  id: innodb_page_undo_log
  title: InnoDB Undo Log Page
  application: MySQL InnoDB Storage Engine
  license: MIT
  endian: le
  doc: |
    Undo log pages are critical components of InnoDB's transaction system, storing
    before-images of modified data to support Multi-Version Concurrency Control (MVCC)
    and transaction rollback. Each undo log segment contains a linked list of undo
    records that capture the state of data before modifications, enabling ACID
    compliance through isolation levels and crash recovery.
  imports:
    - ../innodb_common

seq:
  - id: fil_header
    type: innodb_common::fil_header_t
    doc: |
      Standard InnoDB file page header containing page metadata, checksums,
      and addressing information. Identifies this as an undo log page.
  - id: undo_page_header
    type: undo_page_header_t
    doc: |
      Undo-specific page header containing log record offsets, free space tracking,
      and page linkage information for the undo segment.
  - id: undo_records
    type: undo_record_list_t
    doc: |
      The actual undo records stored on this page. Contains a linked list of
      undo record structures, each representing a before-image of modified data
      for transaction rollback or MVCC read operations.
  - id: fil_trailer
    type: innodb_common::fil_trailer_t
    doc: |
      Standard InnoDB file page trailer providing additional integrity checking
      and recovery information for this undo log page.

types:
  undo_page_header_t:
    doc: |
      Header specific to undo log pages, containing metadata about the undo segment
      and record organization. This header tracks the location of undo records and
      manages space allocation within the undo page.
      Reference: trx0undo.h (struct undo_page_header_t)
    seq:
      - id: page_type
        type: u2
        enum: undo_page_type_enum
        doc: |
          Type of undo page (INSERT or UPDATE). Determines the kind of undo records
          stored on this page and affects how they are processed during rollback.
      - id: latest_log_record_offset
        type: u2
        doc: |
          Offset from page start to the most recently added undo record. This is the
          head of the undo record linked list, used as the starting point for parsing
          all records on the page.
      - id: free_offset
        type: u2
        doc: |
          Offset indicating where new undo records can be added. Tracks available
          space on the page for appending additional undo records as transactions
          generate more undo information.
      - id: page_list_node
        size: 12
        doc: |
          Doubly-linked list pointers connecting this page to other pages in the
          same undo segment. Enables traversal of the entire undo log segment when
          processing transactions that span multiple pages.

  undo_record_t:
    doc: |
      Individual undo record containing before-image data for a single database
      modification. Each record captures the state of data before a change,
      enabling transaction rollback and MVCC read consistency.
      Reference: trx0rec.h (struct undo_rec_t)
    seq:
      - id: undo_rec_type
        type: u1
        enum: undo_record_type_enum
        doc: |
          Type of undo operation this record represents. Determines the structure
          and interpretation of the data payload that follows.
      - id: undo_no
        type: innodb_common::mach_compressed_uint_t
        doc: |
          Unique undo record number within this transaction. Used to order undo
          operations during rollback and ensure correct sequence of operations.
      - id: table_id
        type: innodb_common::mach_compressed_uint_t
        doc: |
          Identifier of the table this undo record belongs to. Essential for
          routing undo operations to the correct table during rollback.
      - id: info_bits
        type: u1
        doc: |
          Additional information flags about the undo record. Contains metadata
          about the record's properties and special handling requirements.
      - id: trx_id
        type: u8
        doc: |
          Transaction ID that created this undo record. Links the undo record
          to its originating transaction for rollback and MVCC operations.
      - id: roll_ptr
        type: u8
        doc: |
          Rollback pointer to the previous undo record in this transaction's chain.
          Enables traversal of the complete undo history for a transaction.
      - id: data
        type:
          switch-on: undo_rec_type
          cases:
            undo_record_type_enum::insert: undo_insert_data_t
            undo_record_type_enum::update_existing: undo_update_data_t
            undo_record_type_enum::update_deleted: undo_update_data_t
            undo_record_type_enum::delete: undo_delete_data_t
            undo_record_type_enum::purge: undo_delete_data_t
            undo_record_type_enum::insert_truncate: undo_truncate_data_t
            undo_record_type_enum::update_truncate: undo_truncate_data_t
        doc: |
          Type-specific undo data payload. The structure varies based on the
          undo record type, containing the actual before-image data needed
          for rollback or MVCC operations.
      - id: next_record_offset
        type: u2
        doc: |
          Offset to the next undo record on this page. Forms a linked list
          of records within the page, allowing sequential processing.

  undo_record_list_t:
    doc: |
      Container for the linked list of undo records on this page. Provides
      access to the first record via the page header's latest_log_record_offset.
    instances:
      first_record:
        pos: _parent.undo_page_header.latest_log_record_offset
        type: undo_record_with_next_t
        if: _parent.undo_page_header.latest_log_record_offset != 0
        doc: |
          The first (most recent) undo record on this page. Serves as the entry
          point for traversing the linked list of all undo records.

  undo_record_with_next_t:
    doc: |
      Wrapper for an undo record that includes navigation to the next record
      in the linked list. Forms the chain of undo records within a page.
    seq:
      - id: record
        type: undo_record_t
        doc: The actual undo record data and metadata.
    instances:
      next_record:
        pos: record.next_record_offset
        type: undo_record_with_next_t
        if: record.next_record_offset != 0 and record.next_record_offset < _root._io.size
        doc: |
          Reference to the next undo record in the chain. Allows traversal
          of the complete undo record linked list on this page.

  # Undo data formats for different record types
  undo_field_data_t:
    doc: |
      Generic field data structure for undo records. Represents a single
      database field with its length and value, used across different
      undo record types.
    seq:
      - id: len_field_value
        type: u4
        doc: |
          Length of the field value in bytes. Determines how much data
          follows in the field_value field.
      - id: field_value
        size: len_field_value
        doc: |
          The actual field data bytes. Content varies by field type
          (integer, string, BLOB, etc.) and represents the before-image
          of the field value.

  undo_insert_data_t:
    doc: |
      Undo data for INSERT operations. Contains the primary key of the
      inserted row, allowing the row to be located and removed during rollback.
    seq:
      - id: primary_key_fields
        type: undo_field_data_t
        repeat: eos
        doc: |
          Primary key field values of the inserted row. Used to identify
          and remove the row during transaction rollback operations.

  undo_update_data_t:
    doc: |
      Undo data for UPDATE operations. Contains the old values of fields
      that were modified, allowing restoration of the original values
      during rollback.
    seq:
      - id: num_field_old_values
        type: u4
        doc: |
          Number of fields that were updated in this operation. Determines
          how many field entries follow in the arrays below.
      - id: field_numbers
        type: innodb_common::mach_compressed_uint_t
        repeat: expr
        repeat-expr: num_field_old_values
        doc: |
          Column positions (field numbers) of the fields that were updated.
          Used to identify which columns need their values restored.
      - id: field_old_values
        type: undo_field_data_t
        repeat: expr
        repeat-expr: num_field_old_values
        doc: |
          The original values of the updated fields before the modification.
          These values are restored during transaction rollback.

  undo_delete_data_t:
    doc: |
      Undo data for DELETE operations. Contains the complete deleted row
      data, allowing the row to be restored during rollback operations.
    seq:
      - id: record_header
        type: undo_record_header_t
        doc: |
          Header information for the deleted row, including field count
          and record metadata.
      - id: null_bitmap
        size: (record_header.num_fields + 7) / 8
        doc: |
          Bitmap indicating which fields in the row are NULL. Each bit
          represents one field, with 1 indicating NULL and 0 indicating
          a non-NULL value.
      - id: field_data
        type: undo_field_data_t
        repeat: until
        repeat-until: _io.pos >= _io.size - 2  # Stop before next_record_offset
        doc: |
          Field values for each column in the deleted row. Contains the
          complete row data needed to restore the deleted record.

  undo_record_header_t:
    doc: |
      Simplified header for undo record data structures. Contains basic
      metadata about the record format and field layout.
    seq:
      - id: info_flags
        type: u1
        doc: |
          Information flags about the record structure and properties.
          Contains metadata about how to interpret the record data.
      - id: num_fields
        type: u4
        doc: |
          Total number of fields in this record. Used to determine the
          size of the null bitmap and expected number of field entries.

  undo_truncate_data_t:
    doc: |
      Undo data for TRUNCATE operations. Contains metadata about the
      truncated table and operation parameters for rollback recovery.
    seq:
      - id: truncate_table_id
        type: innodb_common::mach_compressed_uint_t
        doc: |
          Identifier of the table that was truncated. Used to route
          the truncate operation to the correct table during rollback.
      - id: truncate_flags
        type: u4
        doc: |
          Flags describing the truncate operation characteristics and
          behavior. Contains operation-specific metadata.
      - id: truncate_index_id
        type: u8
        doc: |
          Index identifier associated with the truncate operation.
          May reference specific indexes affected by the truncation.
      - id: truncate_extra_data
        size-eos: true
        doc: |
          Additional operation-specific data for the truncate undo record.
          Contains extra metadata needed for proper rollback of truncate operations.

enums:
  undo_page_type_enum:
    doc: |
      Types of undo pages, determining whether they contain undo records for
      INSERT operations or UPDATE operations. This affects how the undo records
      are processed during transaction rollback and recovery.
      Reference: trx0undo.h (enum undo_page_type)
    1: insert
      doc: |
        Undo page containing records for INSERT operations. These records
        store primary key information needed to locate and remove inserted
        rows during transaction rollback.
    2: update
      doc: |
        Undo page containing records for UPDATE operations. These records
        store before-images of modified data needed to restore original
        values during transaction rollback.

  undo_record_type_enum:
    doc: |
      Specific types of undo operations that can be recorded. Each type
      corresponds to a different kind of database modification and determines
      the structure and content of the undo record data.
      Reference: trx0rec.h (enum undo_rec_type)
    11: insert
      doc: |
        Undo record for an INSERT operation. Contains primary key information
        of the inserted row, allowing the row to be located and removed
        during rollback.
    12: update_existing
      doc: |
        Undo record for updating an existing row. Contains the old values
        of modified fields, allowing restoration of the original data
        during rollback.
    13: update_deleted
      doc: |
        Undo record for updating a row marked as deleted. Similar to update_existing
        but the row was already logically deleted, requiring special handling
        during rollback operations.
    14: delete
      doc: |
        Undo record for a DELETE operation. Contains the complete row data
        of the deleted record, allowing full restoration during rollback.
    15: purge
      doc: |
        Undo record for purge operations. Similar to delete but used during
        cleanup of records that are no longer needed by any active transaction.
    16: insert_truncate
      doc: |
        Undo record for INSERT operations during table truncation. Specialized
        handling for truncate operations that may affect large numbers of rows.
    17: update_truncate
      doc: |
        Undo record for UPDATE operations during table truncation. Specialized
        handling for truncate operations involving data modifications.
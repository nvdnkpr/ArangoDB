/*   ftsindex.h - The Full Text Search header file  */
/*   R. A. Parker    6.6.2012  */

typedef struct FTS_REAL_index FTS_index_t;

typedef struct
{
    uint64_t _globalOptions;
    size_t _len;
    uint64_t * _localOptions;
    uint8_t * * _texts;
}  FTS_query_t;

/* local Options value (one for each word) */

#define FTS_MATCH_COMPLETE 1
#define FTS_MATCH_PREFIX 2
#define FTS_MATCH_SUBSTRING 4
#define FTS_INDEX_SUBSTRINGS 1

typedef struct
{
    size_t _len;
    FTS_document_id_t * _docs;
}  FTS_document_ids_t;

#define FTS_SIZES_DEFAULT {10,1000,57,100,0,0,0,0,0,0}

FTS_index_t * FTS_CreateIndex(FTS_collection_id_t coll,
    uint64_t options, uint64_t sizes[10]);

void FTS_FreeIndex ( FTS_index_t * ftx);

void FTS_BackgroundTask (FTS_index_t * ftx);

void FTS_AddDocument(FTS_index_t * ftx, FTS_document_id_t docid);

void FTS_DeleteDocument(FTS_index_t * ftx, FTS_document_id_t docid);

void FTS_UpdateDocument(FTS_index_t * ftx, FTS_document_id_t docid);

FTS_document_ids_t * FTS_FindDocuments (FTS_index_t * ftx,
                                FTS_query_t * query);

void FTS_Free_Documents(FTS_document_ids_t *);

void indexd(FTS_index_t * ftx);

/* end of ftsindex.h */


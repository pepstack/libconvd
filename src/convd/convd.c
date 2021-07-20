/**
 * convd.c
 */
#include "convd_i.h"

const char * convd_lib_version(const char **_libname)
{
    if (_libname) {
        *_libname = LIBNAME;
    }
    return LIBVERSION;
}



int conv_xmlhead_format(const conv_xmlhead_t *xmlhead, conv_buf_t *output)
{
    char headbuf[128];
    int headlen = snprintf_chkd_V1(headbuf, sizeof(headbuf), "<?xml version=\"%s\" encoding=\"%s\"?>", xmlhead->version, xmlhead->encoding);

    switch (xmlhead->bom) {
    case UCS_NONE_BOM:
        if (cstr_safecopy(output->bufp, output->blen, 0, headbuf, headlen)) {
            return headlen;
        }
        return 0;

    case UCS_UTF8_BOM:
        output->bufp[0] = (char)0xEF;
        output->bufp[1] = (char)0xBB;
        output->bufp[2] = (char)0xBF;
        if (cstr_safecopy(output->bufp, output->blen, 3, headbuf, headlen)) {
            return headlen + 3;
        }
        return 0;

    case UCS_2BE_BOM:
        if ((int)output->blen > headlen * 2 + 2) {
            ub2 be2;
            int i = 0, offcb = 2;
            output->bufp[0] = (char)0xFE;
            output->bufp[1] = (char)0xFF;
            for (; i < headlen; i++) {
                be2 = (ub2) BO_i16_htobe(headbuf[i]);
                memcpy(&output->bufp[i*2 + 2], &be2, sizeof(be2));
                offcb += sizeof(be2);
            }
            return offcb;
        }
        return 0;

    case UCS_2LE_BOM:
        if ((int)output->blen > headlen * 2 + 2) {
            ub2 le2;
            int i = 0, offcb = 2;
            output->bufp[0] = (char)0xFF;
            output->bufp[1] = (char)0xFE;
            for (; i < headlen; i++) {
                le2 = (ub2) BO_i16_htole(headbuf[i]);
                memcpy(&output->bufp[i*2 + 2], &le2, sizeof(le2));
                offcb += sizeof(le2);
            }
            return offcb;
        }
        return 0;

    case UCS_4BE_BOM:
         if ((int)output->blen > headlen * 4 + 4) {
            ub4 be4;
            int i = 0, offcb = 4;
            output->bufp[0] = (char)0x00;
            output->bufp[1] = (char)0x00;
            output->bufp[2] = (char)0xFE;
            output->bufp[3] = (char)0xFF;
            for (; i < headlen; i++) {
                be4 = (ub4) BO_i32_htobe(headbuf[i]);
                memcpy(&output->bufp[i*4 + 4], &be4, sizeof(be4));
                offcb += sizeof(be4);
            }
            return offcb;
        }
        return 0;

    case UCS_4LE_BOM:
        if ((int)output->blen > headlen * 4 + 4) {
            ub4 le4;
            int i = 0, offcb = 4;
            output->bufp[0] = (char)0xFF;
            output->bufp[1] = (char)0xFE;
            output->bufp[2] = (char)0x00;
            output->bufp[3] = (char)0x00;
            for (; i < headlen; i++) {
                le4 = (ub4) BO_i32_htole(headbuf[i]);
                memcpy(&output->bufp[i*4 + 4], &le4, sizeof(le4));
                offcb += sizeof(le4);
            }
            return offcb;
        }
        return 0;
    default:
        break;
    }

    /* unknown bom */
    return -1;    
}


CONVD_UCS_BOM UCS_text_detect_bom(const conv_buf_t *header)
{
    if (header->blen > 1) {
        if ((ub1)header->bufp[0] == 0xFE && (ub1)header->bufp[1] == 0xFF) {
            return UCS_2BE_BOM;
        }

        if ((ub1)header->bufp[0] == 0x00 && (ub1)header->bufp[1] == 0x00) {
            if (header->blen > 3) {
                if ((ub1)header->bufp[2] == 0xFE && (ub1)header->bufp[3] == 0xFF) {
                    return UCS_4BE_BOM;
                }
            }
        }

        if ((ub1)header->bufp[0] == 0xFF && (ub1)header->bufp[1] == 0xFE) {
            if (header->blen > 3) {
                if ((ub1)header->bufp[2] == 0x00 && (ub1)header->bufp[3] == 0x00) {
                    return UCS_4LE_BOM;
                }
            }
            return UCS_2LE_BOM;
        }

        if ((ub1)header->bufp[0] == 0xEF && (ub1)header->bufp[1] == 0xBB) {
            if (header->blen == 2) {
                /* need one more byte */
                return UCS_UTF8_BOM_ASK;
            }

            if ((ub1)header->bufp[2] == 0xBF) {
                return UCS_UTF8_BOM;
            }
        }
    }

    return UCS_NONE_BOM;
}


int UCS_file_detect_bom(const char *textfile, CONVD_UCS_BOM *outbom)
{
    int bom = UCS_NONE_BOM;

    filehandle_t hf = file_open_read(textfile);

    if (hf != filehandle_invalid) {
        conv_buf_t headbuf;
        char header[4] = {0x00, 0x00, 0x00, 0x00};

        /* try to read first 4 bytes */
        int cb = file_readbytes(hf, header, 4);

        bom = UCS_text_detect_bom(convbuf_mk(&headbuf, header, cb));

        file_close(&hf);

        if (bom == UCS_UTF8_BOM_ASK) {
            *outbom = UCS_NONE_BOM;
        } else {
            *outbom = bom;
        }

        return CONVD_NOERROR;
    }

    return CONVD_ERR_FILE;
}


int XML_text_parse_head(const conv_buf_t *xmltext, conv_xmlhead_t *xmlhead)
{
    int found = 0;
    int offset = 0;

    int headlen = 0;
    char headbuf[128];

    memset(xmlhead, 0, sizeof(*xmlhead));
    xmlhead->bom = UCS_text_detect_bom(xmltext);

    switch (xmlhead->bom) {
    case UCS_2BE_BOM:
        offset = 2;
        while(offset + 1 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_2LE_BOM:
        offset = 2;
        while(offset + 1 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i16(&xmltext->bufp[offset]);
                offset += 2;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4BE_BOM:
        offset = 4;
        while(offset + 3 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_betoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;

    case UCS_4LE_BOM:
        offset = 4;
        while(offset + 3 < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = (char) BO_bytes_letoh_i32(&xmltext->bufp[offset]);
                offset += 4;
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        } 
        break;

    case UCS_UTF8_BOM:
        offset = 3;
    case UCS_NONE_BOM:
    default:
        while(offset < (int)xmltext->blen) {
            if (headlen < sizeof(headbuf) - 1) {
                headbuf[headlen] = xmltext->bufp[offset++];
                if (headbuf[headlen++] == '>') {
                    headbuf[headlen] = '\0';
                    found = 1;
                    break;
                }
            }
        }
        break;
    }

    if (found && headlen > 20 && cstr_startwith(headbuf, headlen, "<?xml ", 6) &&
        headbuf[headlen-2] == '?' && headbuf[headlen-1] == '>') {
        char *start, *end, *s;

        start = strstr(headbuf, "version=\"");
        if (start) {
            s = &start[9];
            end = strstr(s, "\"");
            if (end && (int)(end - s) > 1) {
                memcpy(xmlhead->version, s, (end - s));
                xmlhead->version[(end - s)] = 0;
            }

            start = strstr(headbuf, "encoding=\"");
            if (start) {
                s = &start[10];
                end = strstr(s, "\"");
                if (end && (int)(end - s) > 0 && (end - s) < sizeof(xmlhead->encoding)) {
                    memcpy(xmlhead->encoding, s, (end - s));
                    xmlhead->encoding[(end - s)] = 0;
                    cstr_toupper(xmlhead->encoding, (int)(end - s));
                }
            }

            /* 成功: 返回xml头的字节长度 */
            return offset;
        }
    }

    /* xml head not found */
    return 0;
}


int XML_file_parse_head(const char *xmlfile, conv_xmlhead_t *xmlhead)
{
    filehandle_t hf = file_open_read(xmlfile);
    if (hf != filehandle_invalid) {
        conv_buf_t input;
        char headbuf[CVD_ENCODING_LEN_MAX + 16 + 32];

        /* try to read head bytes */
        int headlen = file_readbytes(hf, headbuf, sizeof(headbuf));

        file_close(&hf);

        return XML_text_parse_head(convbuf_mk(&input, headbuf, headlen), xmlhead);
    }
    return CONVD_ERR_FILE;
}


static void convd_cleanup_cb (void *cvd)
{
    if (((conv_descriptor_t *)cvd)->cd != CONVD_ERROR_ICONV) {
        iconv_close(((conv_descriptor_t *)cvd)->cd);
    }
}


conv_buf_t * convbuf_mk(conv_buf_t *cvbuf, char *arraybytes, size_t numbytes)
{
    cvbuf->bufp = arraybytes;
    cvbuf->blen = numbytes;
    return cvbuf;
}


static int ucs_file_read_open(convd_t cvd, const char *pathfile, ub4 linesizemax, convpos_t *_cpos)
{
    filehandle_t hf = file_open_read(pathfile);
    if (hf != filehandle_invalid) {
        convpos_t cpos;

        int bom = UCS_NONE_BOM;
        conv_buf_t headbuf;
        char header[4] = {0x00, 0x00, 0x00, 0x00};

        /* try to read first 4 bytes */
        int offcb = file_readbytes(hf, header, 4);
        bom = UCS_text_detect_bom(convbuf_mk(&headbuf, header, offcb));
        if (bom == UCS_UTF8_BOM_ASK) {
            file_close(&hf);
            return CONVD_ERR_ENCODING;
        }

        offcb = 0;
        if (bom == UCS_2BE_BOM || bom == UCS_2LE_BOM) {
            offcb = 2;
        } else if (bom == UCS_4BE_BOM || bom == UCS_4LE_BOM) {
            offcb = 4;
        } else if (bom == UCS_UTF8_BOM) {
            offcb = 3;
        }
        if (file_seek(hf, offcb, 0) != offcb) {
            file_close(&hf);
            return CONVD_ERR_FILE;
        }

        if (linesizemax == -1) {
             linesizemax = CONV_LINE_MAXSIZE;
        }

        cpos = (convpos_t)mem_alloc_unset(sizeof(conv_position_t) + (5 * linesizemax) * sizeof(char));

        cpos->cvd = cvd;
        cpos->bom = bom;
        cpos->offset = offcb;
        cpos->textfd = hf;
        cpos->linesize = linesizemax;
        cpos->inputbuf = cpos->linebuf;
        cpos->outputbuf = &cpos->linebuf[cpos->linesize];

        *_cpos= cpos;
        return CONVD_NOERROR;
    }

    return CONVD_ERR_FILE;
}


static int ucs_file_read_next(convpos_t cpos)
{
    ub1 chbyte;
    int bitflag;
    int offcb = 0;
    int rdlen = file_readbytes(cpos->textfd, cpos->inputbuf, cpos->linesize);
    if (rdlen == -1) {
        return CONVD_ERR_FILE;
    }
    if (rdlen == 0) {
        return CONV_FINISHED;
    }

    switch (cpos->bom) {
    case UCS_2BE_BOM:
        while (offcb <= rdlen - 2) {
            BO_bytes_betoh(cpos->inputbuf + offcb, 2);
            offcb += 2;
        }
        break;

    case UCS_2LE_BOM:
        while (offcb <= rdlen - 2) {
            BO_bytes_letoh(cpos->inputbuf + offcb, 2);
            offcb += 2;
        }
        break;

    case UCS_4BE_BOM:
        while (offcb <= rdlen - 4) {
            BO_bytes_betoh(cpos->inputbuf + offcb, 4);
            offcb += 4;
        }
        break;

    case UCS_4LE_BOM:
        while (offcb <= rdlen - 4) {
            BO_bytes_letoh(cpos->inputbuf + offcb, 4);
            offcb += 4;
        }
        break;

    case UCS_UTF8_BOM:
    case UCS_NONE_BOM:
    default:
        /* 读到换行符(\n), ascii字符和非ascii字符集转换处结束. 包括换行符. 如果遇到(\r\n), 去掉(\r) */
        chbyte = cpos->inputbuf[offcb++];
        bitflag = BO_check_bit(chbyte, 7);
        while (offcb < rdlen) {
            chbyte = cpos->inputbuf[offcb];
            if ((int)BO_check_bit(chbyte, 7) != bitflag) {
                /* 字符集转换处结束 */
                break;
            }
            offcb++;
        }

        if (offcb == cpos->linesize) {
            /* insufficent buf size for line */
            return CONVD_ERR_INSUFF;
        }
        break;
    }

    cpos->offset += offcb;

    if (file_seek(cpos->textfd, cpos->offset, fseek_pos_set) != cpos->offset) {
        return CONVD_ERR_FILE;
    }

    return offcb;
}


static void ucs_file_read_close(convpos_t cpos)
{
    file_close(&cpos->textfd);
    mem_free(cpos);
}


int convd_create(const char *fromcode, const char *tocode, CONVD_SUFFIX_MODE suffix, convd_t *outcvd)
{
    convd_t cvd;

    int fromlen, tolen;

    fromlen = cstr_length(fromcode, CVD_ENCODING_LEN_MAX + 1);
    if (fromlen < 1 || fromlen > CVD_ENCODING_LEN_MAX) {
        /* invalid from code */
        return CONVD_ERR_FROMCODE;
    }

    tolen = cstr_length(tocode, CVD_ENCODING_LEN_MAX + 1);
    if (tolen < 1 || tolen > CVD_ENCODING_LEN_MAX) {
        /* invalid to code */
        return CONVD_ERR_TOCODE;
    }

    cvd = (conv_descriptor_t *)refc_object_new(0, sizeof(conv_descriptor_t) + fromlen + tolen + sizeof(char) * 12, convd_cleanup_cb);

    cvd->cd = CONVD_ERROR_ICONV;
    cvd->suffix = suffix;

    memcpy(cvd->codebuf, fromcode, fromlen);
    cstr_toupper(cvd->codebuf, fromlen);

    cvd->tocodeat = fromlen + 1;

    if (suffix == CVD_SUFFIX_NONE) {
        memcpy(&cvd->codebuf[cvd->tocodeat], tocode, tolen);
        cstr_toupper(&cvd->codebuf[cvd->tocodeat], tolen);

        cvd->cd = iconv_open(&cvd->codebuf[cvd->tocodeat], cvd->codebuf);
        if (cvd->cd == CONVD_ERROR_ICONV) {
            /* Faile to iconv_open. see: errno */
            refc_object_dec((void**)&cvd);
            return CONVD_ERR_ICONV;
        }

        /* success */
        *outcvd = cvd;
        return CONVD_NOERROR;
    }

    memcpy(&cvd->codebuf[cvd->tocodeat], tocode, tolen);
    cstr_toupper(&cvd->codebuf[cvd->tocodeat], tolen);

    if (suffix == CVD_SUFFIX_IGNORE) {
        memcpy(&cvd->codebuf[cvd->tocodeat + tolen], "//IGNORE", 8);
    } else if (suffix == CVD_SUFFIX_TRANSLIT) {
        memcpy(&cvd->codebuf[cvd->tocodeat + tolen], "//TRANSLIT", 10);
    } else {
        /* Invalid suffix argument: errno = 0 */
        refc_object_dec((void**)&cvd);
        return CONVD_ERR_SUFFIX;
    }

    cvd->cd = iconv_open(&cvd->codebuf[cvd->tocodeat], cvd->codebuf);

    if (cvd->cd == CONVD_ERROR_ICONV) {
        /* Faile to iconv_open. see: errno */
        refc_object_dec((void**)&cvd);
        return CONVD_ERR_ICONV;
    }

    /* success */
    cvd->codebuf[cvd->tocodeat + tolen] = '\0';
    *outcvd = cvd;
    return CONVD_NOERROR;
}


void convd_release(convd_t *pcvd)
{
    refc_object_dec((void**)pcvd);
}


convd_t convd_retain(convd_t *cvd)
{
    return (convd_t)refc_object_inc((void**)cvd);
}


const char * convd_fromcode(const convd_t cvd)
{
    return cvd->codebuf;
}


const char * convd_tocode(const convd_t cvd, CONVD_SUFFIX_MODE *suffix)
{
    if (suffix) {
        *suffix = cvd->suffix;
    }
    return &cvd->codebuf[cvd->tocodeat];
}


int convd_config(const convd_t cvd, int request, void *argument)
{
    int ret;
    refc_object_lock(cvd, 0);
    ret = iconvctl(cvd->cd, request, argument);
    refc_object_unlock(cvd);
    return ret;
}


size_t convd_conv_text(convd_t cvd, conv_buf_t *input, conv_buf_t *output)
{
    size_t ret, outlen;

    refc_object_lock(cvd, 0);

    ret = iconv(cvd->cd, NULL, NULL, NULL, NULL);
    if (ret == CONVD_ERROR_SIZE) {
        refc_object_unlock(cvd);
        return CONVD_ERR_ICONV;
    }

    outlen = output->blen;
    for (;;) {
        ret = iconv(cvd->cd, &input->bufp, &input->blen, &output->bufp, &output->blen);
        if (ret == CONVD_ERROR_SIZE) {
            refc_object_unlock(cvd);
            return CONVD_ERR_ICONV;
        }

        if (input->blen == 0) {
            refc_object_unlock(cvd);

            /* success( >=0 ): all input converted ok */
            return (outlen - output->blen);
        }
    }

    refc_object_unlock(cvd);
    return CONVD_ERR_ICONV;
}


int convd_conv_file(convd_t cvd, const char *textfilein, size_t inoffset, const char *textfileout, const char *outhead, size_t outheadlen, ub4 linesizemax, ub8 *outfilesize)
{
    convpos_t cpos;
    ub8 outfdsize = 0;

    int ret = ucs_file_read_open(cvd, textfilein, linesizemax, &cpos);
    if (ret == CONVD_NOERROR) {
        filehandle_t outfd = file_write_new(textfileout);

        if (outfd != filehandle_invalid) {
            conv_buf_t input, output;

            if (outhead && outheadlen) {
                if (file_writebytes(outfd, outhead, (ub4)outheadlen) == -1) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return CONVD_ERR_FILE;
                }
            }            

            cpos->offset += inoffset;
            file_seek(cpos->textfd, inoffset, fseek_pos_cur);

            while ((ret = ucs_file_read_next(cpos)) > 0) {
                size_t convcb = convd_conv_text(cvd, convbuf_mk(&input, cpos->inputbuf, ret), convbuf_mk(&output, cpos->outputbuf, cpos->linesize * 4));
                if (convcb == CONVD_ERROR_SIZE) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return CONVD_ERR_ICONV;
                }

                if (file_writebytes(outfd, cpos->outputbuf, (ub4)convcb) == -1) {
                    /* error */
                    file_close(&outfd);
                    ucs_file_read_close(cpos);
                    return CONVD_ERR_FILE;
                }

                outfdsize += convcb;
            }

            file_close(&outfd);
            ucs_file_read_close(cpos);

            if (ret == CONV_FINISHED) {
                /* success returns 0 */
                if (outfilesize) {
                    *outfilesize = outfdsize;
                }
            }

            return ret;
        }
        ucs_file_read_close(cpos);
    }
    return ret;
}


size_t convd_conv_xmltext(convd_t cvd, conv_buf_t *input, conv_buf_t *output)
{
    conv_xmlhead_t xmlhead;
    conv_buf_t inbuf, outbuf;

    size_t convsize = 0;
    int offslen = 0;

    int xmlheadlen = XML_text_parse_head(convbuf_mk(&inbuf, input->bufp, input->blen), &xmlhead);

    if (stricmp(convd_fromcode(cvd), xmlhead.encoding)) {
        return CONVD_ERR_ICONV;
    }

    if (! memcmp(convd_tocode(cvd, 0), "UTF-8", 5) ||
        ! memcmp(convd_tocode(cvd, 0), "GB2312", 6) ||
        ! memcmp(convd_tocode(cvd, 0), "BIG5", 4) ||
        ! memcmp(convd_tocode(cvd, 0), "GBK", 3) ||
        ! memcmp(convd_tocode(cvd, 0), "GB18030", 7)) {
        // ansi == utf8
        xmlhead.bom = 0;
        cstr_safecopy(xmlhead.encoding, sizeof(xmlhead.encoding), 0, convd_tocode(cvd, 0), cstr_length(convd_tocode(cvd, 0), sizeof(xmlhead.encoding) - 1));

        offslen = conv_xmlhead_format(&xmlhead, convbuf_mk(&outbuf, output->bufp, output->blen));
    } else {

        // TODO:
        // utf8 -> tocode
    }

    convsize = convd_conv_text(cvd,
                    convbuf_mk(&inbuf, &input->bufp[xmlheadlen], input->blen - xmlheadlen),
                    convbuf_mk(&outbuf, &output->bufp[offslen], output->blen - offslen));
    if (convsize == -1) {
        return CONVD_ERR_ICONV;
    }

    /* success */
    return convsize + (size_t) offslen;
}


int convd_conv_xmlfile(convd_t cvd, const char *xmlfilein, const char *xmlfileout, ub4 linesizemax, int addbom, ub8 *outfilesize)
{
    char xmlheadbuf[128];
    int xmlheadlen = 0;

    conv_xmlhead_t xmlhead;
    int headofflen = XML_file_parse_head(xmlfilein, &xmlhead);
    if (headofflen > 0) {
        if (! memcmp(convd_tocode(cvd, 0), "UTF-8", 5) ||
            ! memcmp(convd_tocode(cvd, 0), "GB2312", 6) ||
            ! memcmp(convd_tocode(cvd, 0), "BIG5", 4) ||
            ! memcmp(convd_tocode(cvd, 0), "GBK", 3) ||
            ! memcmp(convd_tocode(cvd, 0), "GB18030", 7)) {
            conv_buf_t output;

            const char *tocode = convd_tocode(cvd, 0);
            int tocodelen = cstr_length(tocode, -1);

            // ansi == utf8
            xmlhead.bom = 0;
            cstr_safecopy(xmlhead.encoding, sizeof(xmlhead.encoding), 0, tocode, tocodelen);
            xmlheadlen = conv_xmlhead_format(&xmlhead, convbuf_mk(&output, xmlheadbuf, sizeof(xmlheadbuf)));
        } else {
            // TODO:
            // utf8 -> tocode
        }

        return convd_conv_file(cvd, xmlfilein, headofflen, xmlfileout, xmlheadbuf, xmlheadlen, linesizemax, outfilesize);
    }
    return CONVD_ERR_FILE;
}

static void vi_fetch_filter16(struct ccvg* res, uint32_t fboffset, uint32_t cur_x, union vi_reg_ctrl ctrl, uint32_t vres, uint32_t fetchstate);
static void vi_fetch_filter32(struct ccvg* res, uint32_t fboffset, uint32_t cur_x, union vi_reg_ctrl ctrl, uint32_t vres, uint32_t fetchstate);

static void (*vi_fetch_filter_func[2])(struct ccvg*, uint32_t, uint32_t, union vi_reg_ctrl, uint32_t, uint32_t) =
{
    vi_fetch_filter16, vi_fetch_filter32
};

void (*vi_fetch_filter_ptr)(struct ccvg*, uint32_t, uint32_t, union vi_reg_ctrl, uint32_t, uint32_t);

static void vi_fetch_filter16(struct ccvg* res, uint32_t fboffset, uint32_t cur_x, union vi_reg_ctrl ctrl, uint32_t hres, uint32_t fetchstate)
{
    int r, g, b;
    uint32_t idx = (fboffset >> 1) + cur_x;
    uint8_t hval;
    uint16_t pix;
    uint32_t cur_cvg;
    if (ctrl.aa_mode <= VI_AA_RESAMP_EXTRA)
    {
        PAIRREAD16(pix, hval, idx);
        cur_cvg = ((pix & 1) << 2) | hval;
    }
    else
    {
        RREADIDX16(pix, idx);
        cur_cvg = 7;
    }
    r = GET_HI(pix);
    g = GET_MED(pix);
    b = GET_LOW(pix);

    if (cur_cvg == 7)
    {
        if (ctrl.dither_filter_enable)
            restore_filter16(&r, &g, &b, fboffset, cur_x, hres, fetchstate);
    }
    else
    {
        video_filter16(&r, &g, &b, fboffset, cur_x, hres, cur_cvg, fetchstate);
    }


    res->r = r;
    res->g = g;
    res->b = b;
    res->cvg = cur_cvg;
}

static void vi_fetch_filter32(struct ccvg* res, uint32_t fboffset, uint32_t cur_x, union vi_reg_ctrl ctrl, uint32_t hres, uint32_t fetchstate)
{
    int r, g, b;
    uint32_t cur_cvg;
    uint32_t pix, addr = (fboffset >> 2) + cur_x;

    RREADIDX32(pix, addr);
    if (ctrl.aa_mode <= VI_AA_RESAMP_EXTRA)
        cur_cvg = (pix >> 5) & 7;
    else
        cur_cvg = 7;
    r = (pix >> 24) & 0xff;
    g = (pix >> 16) & 0xff;
    b = (pix >> 8) & 0xff;

    if (cur_cvg == 7)
    {
        if (ctrl.dither_filter_enable)
            restore_filter32(&r, &g, &b, fboffset, cur_x, hres, fetchstate);
    }
    else
    {
        video_filter32(&r, &g, &b, fboffset, cur_x, hres, cur_cvg, fetchstate);
    }

    res->r = r;
    res->g = g;
    res->b = b;
    res->cvg = cur_cvg;
}

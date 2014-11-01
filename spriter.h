inline float DegToRad( float deg )
{
	return (float)(deg / 180.f) * (float)M_PI;
}

inline float WrapAngle( float ang )
{
	while ( ang < -M_PI )
		ang += 2.f * (float)M_PI;
	while ( ang > M_PI )
		ang -= 2.f * (float)M_PI;
	return ang;
}

inline float InterpAngle( float to, float from, float dt )
{
	float wrap = WrapAngle( to - from );
	return from + wrap * dt;
}

struct Matrix2x3
{
	float f[3][2];
};

void mul( Matrix2x3 &o, Matrix2x3 const &a, Matrix2x3 const &b )
{
	o.f[0][0] = a.f[0][0] * b.f[0][0] + a.f[0][1] * b.f[1][0];
	o.f[0][1] = a.f[0][0] * b.f[0][1] + a.f[0][1] * b.f[1][1];
	o.f[1][0] = a.f[1][0] * b.f[0][0] + a.f[1][1] * b.f[1][0];
	o.f[1][1] = a.f[1][0] * b.f[0][1] + a.f[1][1] * b.f[1][1];
	o.f[2][0] = a.f[2][0] * b.f[0][0] + a.f[2][1] * b.f[1][0] + b.f[2][0];
	o.f[2][1] = a.f[2][0] * b.f[0][1] + a.f[2][1] * b.f[1][1] + b.f[2][1];
}

void mul( float o[2], float x, float y, Matrix2x3 const &a )
{
	o[0] = x * a.f[0][0] + y * a.f[1][0] + a.f[2][0];
	o[1] = x * a.f[0][1] + y * a.f[1][1] + a.f[2][1];
}

void Serialise( Json::Value const &node, bool &val, bool def = false )
{
	if ( node.isNull() )
	{
		val = def;
		return;
	}
	val = node.asBool();
}

void Serialise( Json::Value const &node, int &val, int def = 0 )
{
	if ( node.isNull() )
	{
		val = def;
		return;
	}
	val = node.asInt();
}

void Serialise( Json::Value const &node, float &val, float def = 0.f )
{
	if ( node.isNull() )
	{
		val = def;
		return;
	}
	val = node.asFloat();
}

void Serialise( Json::Value const &node, std::string &val )
{
	if ( node.isNull() )
	{
		val = "";
		return;
	}
	val = node.asString();
}

template <class T>
void Serialise( Json::Value const &node, std::map<std::string,T> &val )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	unsigned int numKeys = node.getNumKeys();
	for (unsigned int i=0; i<numKeys; i++)
	{
		Serialise( node[node.getKey(i)], val[node.getKey(i)] );
	}
}

template <class T>
void Serialise( Json::Value const &node, std::vector<std::pair<std::string,T>> &val )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	unsigned int numKeys = node.getNumKeys();
	for (unsigned int i=0; i<numKeys; i++)
	{
		val.push_back( std::pair<std::string,T>() );
		std::pair<std::string,T> &kv = val.back();
		kv.first = node.getKey(i);
		Serialise( node[node.getKey(i)], kv.second );
	}
}

template <class T>
void Serialise( Json::Value const &node, std::vector<T> &val )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	int num = node.size();
	for (int i=0; i<num; i++)
	{
		val.push_back( T() );
		T &v = val.back();
		Serialise( node[i], v );
	}
}

template <class T, class RootT>
void Serialise( Json::Value const &node, std::vector<std::pair<std::string,T>> &val, RootT &root )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	unsigned int numKeys = node.getNumKeys();
	for (unsigned int i=0; i<numKeys; i++)
	{
		val.push_back( std::pair<std::string,T>() );
		std::pair<std::string,T> &kv = val.back();
		kv.first = node.getKey(i);
		Serialise( node[node.getKey(i)], kv.second, root );
	}
}

template <class T, class RootT>
void Serialise( Json::Value const &node, std::map<std::string,T> &val, RootT &root )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	unsigned int numKeys = node.getNumKeys();
	for (unsigned int i=0; i<numKeys; i++)
	{
		Serialise( node[node.getKey(i)], val[node.getKey(i)], root );
	}
}


template <class T, class RootT>
void Serialise( Json::Value const &node, std::vector<T> &val, RootT &root  )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	int num = node.size();
	for (int i=0; i<num; i++)
	{
		val.push_back( T() );
		T &v = val.back();
		Serialise( node[i], v, root );
	}
}

template <class T, class RootT, class Root2T>
void Serialise( Json::Value const &node, std::vector<T> &val, RootT &root, Root2T &root2 )
{
	if ( node.isNull() )
	{
		val.clear();
		return;
	}
	int num = node.size();
	for (int i=0; i<num; i++)
	{
		val.push_back( T() );
		T &v = val.back();
		Serialise( node[i], v, root, root2 );
	}
}

struct PosColorTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;
	float m_u;
	float m_v;
};

static bgfx::VertexDecl s_PosColorTexCoord0Decl;

static const char* s_shaderPath = NULL;
static bool s_flipV = false;

static void shaderFilePath(char* _out, const char* _name)
{
	strcpy(_out, s_shaderPath);
	strcat(_out, _name);
	strcat(_out, ".bin");
}

long int fsize(FILE* _file)
{
	long int pos = ftell(_file);
	fseek(_file, 0L, SEEK_END);
	long int size = ftell(_file);
	fseek(_file, pos, SEEK_SET);
	return size;
}

static const bgfx::Memory* load(const char* _filePath)
{
	FILE* file = fopen(_filePath, "rb");
	if (NULL != file)
	{
		uint32_t size = (uint32_t)fsize(file);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		size_t ignore = fread(mem->data, 1, size, file);
		BX_UNUSED(ignore);
		fclose(file);
		mem->data[mem->size-1] = '\0';
		return mem;
	}

	return NULL;
}

static const bgfx::Memory* loadShader(const char* _name)
{
	char filePath[512];
	shaderFilePath(filePath, _name);
	return load(filePath);
}

static const bgfx::Memory* loadTexture_(const char* _name, bool rel=false)
{
	char filePath[512];
	if ( _name[0] == '/' || _name[0] == '\\'  )
	{
		strcpy(filePath, _name+1);
	}
	else if ( rel )
	{
		strcpy(filePath, _name);
	} else
	{
		strcpy(filePath, "textures/");
		strcat(filePath, _name);
	}
	return load(filePath);
}


bgfx::TextureHandle LoadTexture( const char *filename, bgfx::TextureInfo &ti )
{
	static std::map<std::string,std::pair<bgfx::TextureHandle,bgfx::TextureInfo>> cache;

	std::map<std::string,std::pair<bgfx::TextureHandle,bgfx::TextureInfo>>::iterator f = cache.find( filename );
	if ( f != cache.end() )
	{
		ti = f->second.second;
		return f->second.first;
	}

	bgfx::TextureHandle textureColor;
	const bgfx::Memory* mem;
	mem = loadTexture_( filename, true );
	textureColor = bgfx::createTexture(mem, 0, &ti);
	cache[filename].first = textureColor;
	cache[filename].second = ti;
	return textureColor;
}

struct NSVGImage* LoadSVG( const char *filename )
{
	static std::map<std::string,struct NSVGImage*> cache;

	std::map<std::string,struct NSVGImage*>::iterator f = cache.find( filename );
	if ( f != cache.end() )
	{
		return f->second;
	}
	const bgfx::Memory* mem;
	mem = load( filename );
	struct NSVGImage* svg = nsvgParse((char*)mem->data);
	cache[filename] = svg;
	return svg;
}


static char *readTextFile( const char *filename, int &len )
{
	FILE *f = fopen( filename, "rb" );
	if ( f )
	{
		fseek( f, 0, SEEK_END );
		len = ftell( f );
		fseek( f, 0, SEEK_SET );
		char *mem = (char*)malloc( len + 1 );
		mem[len] = 0;
		fread( mem, 1, len, f );
		fclose( f );
		return mem;
	}
	return NULL;
}

static bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
{
	const bgfx::Memory* mem;

	// Load vertex shader.
	mem = loadShader(_vsName);
	bgfx::VertexShaderHandle vsh = bgfx::createVertexShader(mem);

	// Load fragment shader.
	mem = loadShader(_fsName);
	bgfx::FragmentShaderHandle fsh = bgfx::createFragmentShader(mem);

	// Create program from shaders.
	bgfx::ProgramHandle program = bgfx::createProgram(vsh, fsh);

	// We can destroy vertex and fragment shader here since
	// their reference is kept inside bgfx after calling createProgram.
	// Vertex and fragment shader will be destroyed once program is
	// destroyed.
	bgfx::destroyVertexShader(vsh);
	bgfx::destroyFragmentShader(fsh);

	return program;
}

bool allocTransientBuffers(bgfx::TransientVertexBuffer* _tvb, const bgfx::VertexDecl& _decl, uint16_t _numVertices, bgfx::TransientIndexBuffer* _tib, uint16_t _numIndices)
{
	if (bgfx::checkAvailTransientVertexBuffer(_numVertices, _decl)
	&&  bgfx::checkAvailTransientIndexBuffer(_numIndices) )
	{
		bgfx::allocTransientVertexBuffer(_tvb, _numVertices, _decl);
		bgfx::allocTransientIndexBuffer(_tib, _numIndices);
		return true;
	}

	return false;
}

void renderScreenSpaceQuad(uint32_t _view, bgfx::ProgramHandle _program, float _x, float _y, float _width, float _height)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, 4, &tib, 6) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		const float minx = _x;
		const float maxx = _x + _width;
		const float miny = _y;
		const float maxy = _y + _height;

		float minu =  0.0f;
		float minv =  0.0f;
		float maxu =  1.0f;
		float maxv =  1.0f;

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_abgr = 0xff0000ff;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_abgr = 0xff00ff00;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_abgr = 0xffff0000;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		vertex[3].m_x = minx;
		vertex[3].m_y = maxy;
		vertex[3].m_z = zz;
		vertex[3].m_abgr = 0xffffffff;
		vertex[3].m_u = minu;
		vertex[3].m_v = maxv;

		uint16_t* indices = (uint16_t*)tib.data;

		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;
		indices[3] = 0;
		indices[4] = 3;
		indices[5] = 2;

		bgfx::setProgram(_program);
		//bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceQuad(uint32_t _view, bgfx::ProgramHandle _program, float _x, float _y, float _width, float _height, float minu, float minv, float maxu, float maxv, unsigned int argb)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, 4, &tib, 6) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		const float minx = _x;
		const float maxx = _x + _width;
		const float miny = _y;
		const float maxy = _y + _height;

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_abgr = argb;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_abgr = argb;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_abgr = argb;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		vertex[3].m_x = minx;
		vertex[3].m_y = maxy;
		vertex[3].m_z = zz;
		vertex[3].m_abgr = argb;
		vertex[3].m_u = minu;
		vertex[3].m_v = maxv;

		uint16_t* indices = (uint16_t*)tib.data;

		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;
		indices[3] = 0;
		indices[4] = 3;
		indices[5] = 2;

		bgfx::setProgram(_program);
		//bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceQuad(uint32_t _view, bgfx::ProgramHandle _program, float _pnts[4][2], float minu, float minv, float maxu, float maxv, unsigned int argb)
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, 4, &tib, 6) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		vertex[0].m_x = _pnts[0][0];
		vertex[0].m_y = _pnts[0][1];
		vertex[0].m_z = zz;
		vertex[0].m_abgr = argb;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = _pnts[1][0];
		vertex[1].m_y = _pnts[1][1];
		vertex[1].m_z = zz;
		vertex[1].m_abgr = argb;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = _pnts[2][0];
		vertex[2].m_y = _pnts[2][1];
		vertex[2].m_z = zz;
		vertex[2].m_abgr = argb;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		vertex[3].m_x = _pnts[3][0];
		vertex[3].m_y = _pnts[3][1];
		vertex[3].m_z = zz;
		vertex[3].m_abgr = argb;
		vertex[3].m_u = minu;
		vertex[3].m_v = maxv;

		uint16_t* indices = (uint16_t*)tib.data;

		indices[0] = 0;
		indices[1] = 2;
		indices[2] = 1;
		indices[3] = 0;
		indices[4] = 3;
		indices[5] = 2;

		bgfx::setProgram(_program);
		//bgfx::setState(BGFX_STATE_DEFAULT);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceGrid(uint32_t _view, bgfx::ProgramHandle _program, float *_pnts, int gx, int gy, unsigned int argb)
{
	int numPnts = gx * gy;
	int numQuads = (gx-1)*(gy-1);
	int numTris = numQuads * 2;
	int numIndices = numTris * 3;
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, numPnts, &tib, numIndices) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		for (int i=0; i<numPnts; i++)
		{
			vertex[i].m_x = _pnts[i*4+0];
			vertex[i].m_y = _pnts[i*4+1];
			vertex[i].m_z = zz;
			vertex[i].m_abgr = argb;
			vertex[i].m_u = _pnts[i*4+2];
			vertex[i].m_v = _pnts[i*4+3];
		}

		uint16_t* indices = (uint16_t*)tib.data;
		uint16_t* curIndices = indices;

		for (int y=1; y<gy; y++)
		{
			for (int x=1; x<gx; x++)
			{
				int px = x-1;
				int py = y-1;
				int i00 = py * gx + px;
				int i01 = py * gx +  x;
				int i10 =  y * gx + px;
				int i11 =  y * gx +  x;
				
				*curIndices++ = i00;
				*curIndices++ = i01;
				*curIndices++ = i11;
				*curIndices++ = i00;
				*curIndices++ = i11;
				*curIndices++ = i10;
			}
		}

		bgfx::setProgram(_program);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceTris(uint32_t _view, bgfx::ProgramHandle _program, float *_pnts, unsigned short *_indices, unsigned int argb, int _numPnts, int _numIndices )
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, _numPnts, &tib, _numIndices) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		for (int i=0; i<_numPnts; i++)
		{
			vertex[i].m_x = _pnts[i*2+0];
			vertex[i].m_y = _pnts[i*2+1];
			vertex[i].m_z = zz;
			vertex[i].m_abgr = argb;
			vertex[i].m_u = 0.f;//_pnts[i*4+2];
			vertex[i].m_v = 0.f;//_pnts[i*4+3];
		}

		uint16_t* indices = (uint16_t*)tib.data;
		memcpy(indices, _indices, sizeof(uint16_t)*_numIndices);

		bgfx::setProgram(_program);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceTrisUV(uint32_t _view, bgfx::ProgramHandle _program, float *_pnts, unsigned short *_indices, unsigned int argb, int _numPnts, int _numIndices )
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, _numPnts, &tib, _numIndices) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		for (int i=0; i<_numPnts; i++)
		{
			vertex[i].m_x = _pnts[i*4+0];
			vertex[i].m_y = _pnts[i*4+1];
			vertex[i].m_z = zz;
			vertex[i].m_abgr = argb;
			vertex[i].m_u = _pnts[i*4+2];
			vertex[i].m_v = _pnts[i*4+3];
		}

		uint16_t* indices = (uint16_t*)tib.data;
		memcpy(indices, _indices, sizeof(uint16_t)*_numIndices);

		bgfx::setProgram(_program);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

void renderScreenSpaceTris(uint32_t _view, bgfx::ProgramHandle _program, float *_pnts, unsigned int argb, int _numPnts )
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	int _numIndices = (_numPnts-2)*3;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, _numPnts, &tib, _numIndices ) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		uint16_t* indices = (uint16_t*)tib.data;
		for (int i=2; i<_numPnts; i++)
		{
			*indices++ = 0;
			*indices++ = i-1;
			*indices++ = i;
		}
		for (int i=0; i<_numPnts; i++)
		{
			vertex[i].m_x = _pnts[i*2+0];
			vertex[i].m_y = _pnts[i*2+1];
			vertex[i].m_z = zz;
			vertex[i].m_abgr = argb;
			vertex[i].m_u = 0.f;//_pnts[i*4+2];
			vertex[i].m_v = 0.f;//_pnts[i*4+3];
		}

		bgfx::setProgram(_program);
		bgfx::setState(0
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}


void renderScreenSpaceLines(uint32_t _view, bgfx::ProgramHandle _program, float *_pnts, unsigned int argb, int _numPnts )
{
	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	int _numIndices = (_numPnts-1)*2;

	if (allocTransientBuffers(&tvb, s_PosColorTexCoord0Decl, _numPnts, &tib, _numIndices ) )
	{
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)tvb.data;

		float zz = 0.0f;

		uint16_t* indices = (uint16_t*)tib.data;
		for (int i=1; i<_numPnts; i++)
		{
			*indices++ = i-1;
			*indices++ = i;
		}
		for (int i=0; i<_numPnts; i++)
		{
			vertex[i].m_x = _pnts[i*2+0];
			vertex[i].m_y = _pnts[i*2+1];
			vertex[i].m_z = zz;
			vertex[i].m_abgr = argb;
			vertex[i].m_u = 0.f;//_pnts[i*4+2];
			vertex[i].m_v = 0.f;//_pnts[i*4+3];
		}

		bgfx::setProgram(_program);
		bgfx::setState(0
			| BGFX_STATE_PT_LINES
			| BGFX_STATE_RGB_WRITE
			| BGFX_STATE_ALPHA_WRITE
			| (BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_INV_SRC_ALPHA))
			);
		bgfx::setIndexBuffer(&tib);
		bgfx::setVertexBuffer(&tvb);
		bgfx::submit(_view);
	}
}

struct TexturePackerRect
{
	int x, y;
	int w, h;
};

struct TexturePackerWH
{
	int w, h;
};

struct TexturePackerFrame
{
	TexturePackerRect frame;
	bool rotated;
	bool trimmed;
	TexturePackerRect spriteSourceSize;
	TexturePackerWH	 sourceSize;
};

struct TexturePacker
{
	std::map<std::string,TexturePackerFrame> frames;
};

void Serialise( Json::Value const &node, TexturePackerRect &rect )
{
	Serialise( node["x"], rect.x );
	Serialise( node["y"], rect.y );
	Serialise( node["w"], rect.w );
	Serialise( node["h"], rect.h );
}

void Serialise( Json::Value const &node, TexturePackerWH &wh )
{
	Serialise( node["w"], wh.w );
	Serialise( node["h"], wh.h );
}

void Serialise( Json::Value const &node, TexturePackerFrame &frame )
{
	Serialise( node["frame"], frame.frame );
	Serialise( node["rotated"], frame.rotated );
	Serialise( node["trimmed"], frame.trimmed );
	Serialise( node["spriteSourceSize"], frame.spriteSourceSize );
	Serialise( node["sourceSize"], frame.sourceSize );

	frame.spriteSourceSize.y = frame.sourceSize.h-(frame.spriteSourceSize.y+frame.spriteSourceSize.h);
}

void Serialise( Json::Value const &node, TexturePacker &packer )
{
	Serialise( node["frames"], packer.frames );
}


#if 1
struct SpriterFile
{
	std::string name;
	float pivotX, pivotY;
	float width, height;
};

struct SpriterFolder
{
	std::string name;
	std::vector<SpriterFile> files;
};

struct SpriterMapInstruction
{
	int folder;
	int file;
	int tarFolder;
	int tarFile;
};

struct SpriterCharacterMap
{
	std::string name;
	std::vector<SpriterMapInstruction> maps;
};

struct SpriterRef
{
	int parent;
	int timeline;
	int key;
	int skin;
	int s;
};

struct SpriterMainlineKey
{
	int time;
	std::vector<SpriterRef> boneRefs;
	std::vector<SpriterRef> objectRefs;
};

struct SpriterSpacial
{
	float x, y;
	float angle;
	float sX, sY;
};

static void mul( SpriterSpacial &out, SpriterSpacial const &a, SpriterSpacial const &parentInfo )
{
	out.angle = a.angle + parentInfo.angle;
    out.sX = a.sX * parentInfo.sX;
    out.sY = a.sY * parentInfo.sY;

    if(a.x!=0||a.y!=0)  
    {
        float preMultX=a.x*parentInfo.sX;
        float preMultY=a.y*parentInfo.sY;
        float s = sin(parentInfo.angle);
        float c = cos(parentInfo.angle);
        out.x = (preMultX * c) - (preMultY * s) + parentInfo.x;
        out.y = (preMultX * s) + (preMultY * c) + parentInfo.y;
    }
    else 
    {
        out.x=parentInfo.x;
        out.y=parentInfo.y;
    }
}

static void lerp( SpriterSpacial &out, SpriterSpacial const &a, SpriterSpacial const &b, float dt )
{
	float invdt = 1.f - dt;
	out.x = invdt * a.x + dt * b.x;
	out.y = invdt * a.y + dt * b.y;
	out.sX = invdt * a.sX + dt * b.sX;
	out.sY = invdt * a.sY + dt * b.sY;
	out.angle = InterpAngle( b.angle, a.angle, dt );
}

struct SpriterSkinPnt
{
	float x, y;
	float u, v;
};

struct SpriterSkin
{
	std::vector<SpriterSkinPnt> skinpnts;
	int dim_x, dim_y;
};

struct SpriterTimelineKey
{
	int time;
	SpriterSpacial spacial;
	std::vector<SpriterSkin> skin;
	int folder;
	int file;
	float pivot[2];
};

struct SpriterTimeline
{
	std::string name;
	int objectType;
	std::vector<SpriterTimelineKey> keys;
};

struct SpriterAnimation
{
	std::string name;
	float length;
	bool looping;
	std::vector<SpriterMainlineKey> mainlineKeys;
	std::vector<SpriterTimeline> timelines;
};

struct SpriterObjInfo
{
	std::string name;
	int w, h;
};

struct SpriterEntity
{
	std::string name;
	std::vector<SpriterObjInfo> boneObjInfo;
	std::vector<SpriterCharacterMap> characterMaps;
	std::vector<SpriterAnimation> animations;
};

struct SpriterObject
{
	unsigned int maxBones;
	std::vector<SpriterFolder> folders;
	std::vector<SpriterEntity> entities;
	std::vector<SpriterFolder> activeCharacterMap;
};

void Serialise( Json::Value const &node, SpriterFile &spriteFile )
{
	Serialise( node["name"], spriteFile.name );
	Serialise( node["pivot_x"], spriteFile.pivotX );
	Serialise( node["pivot_y"], spriteFile.pivotY );
	Serialise( node["width"], spriteFile.width );
	Serialise( node["height"], spriteFile.height );
}

void Serialise( Json::Value const &node, SpriterFolder &spriteFolder )
{
	Serialise( node["name"], spriteFolder.name );
	Serialise( node["file"], spriteFolder.files );
}

void Serialise( Json::Value const &node, SpriterSkinPnt &pnt )
{
	Serialise( node["u"], pnt.u );
	Serialise( node["v"], pnt.v );
	Serialise( node["x"], pnt.x );
	Serialise( node["y"], pnt.y );
}

void Serialise( Json::Value const &node, SpriterSkin &skin )
{
	Json::Value skinpnts = node["xy"];
	for (unsigned int i=0 ;i<skinpnts.size(); i++)
	{
		Serialise( skinpnts[i], skin.skinpnts );
	}
	skin.dim_y = skinpnts.size();
	skin.dim_x = skin.skinpnts.size() / skin.dim_y;
}

void Serialise( Json::Value const &node, SpriterTimelineKey &spriteTimelineKey )
{
	Serialise( node["time"], spriteTimelineKey.time );
	Json::Value const &bone = node["bone"];
	spriteTimelineKey.pivot[0] = FLT_MAX;
	spriteTimelineKey.pivot[1] = FLT_MAX;
	if ( !bone.isNull() )
	{
		Serialise( bone["x"], spriteTimelineKey.spacial.x );
		Serialise( bone["y"], spriteTimelineKey.spacial.y );
		Serialise( bone["scale_x"], spriteTimelineKey.spacial.sX, 1.f );
		Serialise( bone["scale_y"], spriteTimelineKey.spacial.sY, 1.f );
		Serialise( bone["angle"], spriteTimelineKey.spacial.angle );
		spriteTimelineKey.file = -1;
		spriteTimelineKey.folder = -1;
	}
	Json::Value const &object = node["object"];
	if ( !object.isNull() )
	{
		Serialise( object["x"], spriteTimelineKey.spacial.x, 0.f );
		Serialise( object["y"], spriteTimelineKey.spacial.y, 0.f );
		Serialise( object["scale_x"], spriteTimelineKey.spacial.sX, 1.f );
		Serialise( object["scale_y"], spriteTimelineKey.spacial.sY, 1.f );
		Serialise( object["angle"], spriteTimelineKey.spacial.angle, 0.f );
		Serialise( object["file"], spriteTimelineKey.file );
		Serialise( object["folder"], spriteTimelineKey.folder );
		Serialise( object["pivot_x"], spriteTimelineKey.pivot[0], FLT_MAX );
		Serialise( object["pivot_y"], spriteTimelineKey.pivot[1], FLT_MAX );
		Json::Value const &skin = object["skin"];
		if ( !skin.isNull() )
		{
			Serialise( skin["s"], spriteTimelineKey.skin );
		}
	}
	spriteTimelineKey.spacial.angle = DegToRad( spriteTimelineKey.spacial.angle );
}

void Serialise( Json::Value const &node, SpriterTimeline &spriteTimeline, SpriterAnimation &anim )
{
	Serialise( node["name"], spriteTimeline.name );
	Serialise( node["key"], spriteTimeline.keys );
	spriteTimeline.keys.push_back( spriteTimeline.keys[0] );
	spriteTimeline.keys.back().time = (int)anim.length;
}

void Serialise( Json::Value const &node, SpriterRef &ref )
{
	Serialise( node["parent"], ref.parent, -1 );
	Serialise( node["timeline"], ref.timeline, -1 );
	Serialise( node["key"], ref.key );
	Serialise( node["skin"], ref.skin, -1 );
	Serialise( node["s"], ref.s, -1 );
}

void Serialise( Json::Value const &node, SpriterMainlineKey &spriterMainline, SpriterObject &spriter )
{
	Serialise( node["time"], spriterMainline.time );
	Serialise( node["bone_ref"], spriterMainline.boneRefs );
	Serialise( node["object_ref"], spriterMainline.objectRefs );
	if ( spriterMainline.boneRefs.size() > spriter.maxBones )
		spriter.maxBones = spriterMainline.boneRefs.size();
}

void Serialise( Json::Value const &node, SpriterAnimation &spriteAnimation, SpriterObject &spriter )
{
	Serialise( node["name"], spriteAnimation.name );
	Serialise( node["length"], spriteAnimation.length );
	Serialise( node["looping"], spriteAnimation.looping, true );
	Serialise( node["mainline"]["key"], spriteAnimation.mainlineKeys, spriter );
	Serialise( node["timeline"], spriteAnimation.timelines, spriteAnimation );
}

void Serialise( Json::Value const &node, SpriterObjInfo &spriterObjInfo )
{
	Serialise( node["name"], spriterObjInfo.name );
	Serialise( node["w"], spriterObjInfo.w );
	Serialise( node["h"], spriterObjInfo.h );
}

void Serialise( Json::Value const &node, SpriterEntity &spriteEntity, SpriterObject &spriter )
{
	Serialise( node["name"], spriteEntity.name );
	Serialise( node["animation"], spriteEntity.animations, spriter );
}

void Serialise( Json::Value const &node, SpriterObject &spriter )
{
	spriter.maxBones = 0;
	Serialise( node["folder"], spriter.folders );
	Serialise( node["entity"], spriter.entities, spriter );
}

bool SampleAnimation( SpriterSpacial *local, SpriterAnimation &anim, float t )
{
	float ms = anim.looping ? fmodf( t * 1000.f, anim.length ) : t*1000.f;
	unsigned int mainlineIndex = 0;
	while ( (mainlineIndex+1) < anim.mainlineKeys.size() && ms > anim.mainlineKeys[mainlineIndex+1].time )
		mainlineIndex++;
	SpriterMainlineKey &mlkey = anim.mainlineKeys[mainlineIndex];
	for (unsigned int i=0; i<mlkey.boneRefs.size(); i++)
	{
		SpriterRef &ref = mlkey.boneRefs[i];
		SpriterTimeline &timeline = anim.timelines[ref.timeline];
		SpriterSpacial &sample = local[i];
		sample = timeline.keys[0].spacial;

		for (unsigned int j=1; j<timeline.keys.size(); j++)
		{
			SpriterTimelineKey &pk = timeline.keys[j-1];
			SpriterTimelineKey &ck = timeline.keys[j];
			if ( ms >= pk.time && ms < ck.time )
			{
				float dt = (ms - pk.time) / (ck.time-pk.time);
				lerp( sample, pk.spacial, ck.spacial, dt );
				break;
			}
		}
	}
	return !anim.looping && (ms > anim.length);
}

bool SampleAnimation( SpriterSpacial *local, SpriterEntity &ent, std::string const &animName, float t )
{
	for (unsigned int i=0; i<ent.animations.size(); i++)
	{
		if ( ent.animations[i].name != animName )
			continue;

		return SampleAnimation( local, ent.animations[i], t );
	}
	return false;
}

bool SampleAnimation( SpriterSpacial *local, SpriterObject &spriter, std::string const &entityName, std::string const &animName, float t )
{
	for (unsigned int i=0; i<spriter.entities.size(); i++)
	{
		if ( spriter.entities[i].name != entityName )
			continue;
		return SampleAnimation( local, spriter.entities[i], animName, t );
	}
	return false;
}

void LocalToWorld( SpriterSpacial *world, SpriterSpacial *local, SpriterAnimation &anim, float /*t*/ )
{
	int mainlineIndex = 0;
	SpriterMainlineKey &mlkey = anim.mainlineKeys[mainlineIndex];

	for (unsigned int i=0; i<mlkey.boneRefs.size(); i++)
	{
		SpriterRef &ref = mlkey.boneRefs[i];
		if ( ref.parent == -1 )
		{
			world[i] = local[i];
			continue;
		}

		mul( world[i], local[i], world[ref.parent] );
	}
}

void LocalToWorld( SpriterSpacial *world, SpriterSpacial *local, SpriterEntity &ent, std::string const &animName, float t )
{
	for (unsigned int i=0; i<ent.animations.size(); i++)
	{
		if ( ent.animations[i].name != animName )
			continue;

		LocalToWorld( world, local, ent.animations[i], t );
		break;
	}
}

void LocalToWorld( SpriterSpacial *world, SpriterSpacial *local, SpriterObject &spriter, std::string const &entityName, std::string const &animName, float t )
{
	for (unsigned int i=0; i<spriter.entities.size(); i++)
	{
		if ( spriter.entities[i].name != entityName )
			continue;
		LocalToWorld( world, local, spriter.entities[i], animName, t );
		break;
	}
}

inline int Max( int a, int b )
{
	return (a>b) ? a : b;
}

inline int Min( int a, int b )
{
	return (a<b) ? a : b;
}

inline void CatmullRom( float *o, const float *pnts, int numAttr, int numPnts, float t )
{
	float scaledT = t * (numPnts-1);
	int index = (int)floorf(scaledT);
	float x = scaledT - (float)index;
	float w0 = x*((2.f-x)*x-1.f);
	float w1 = x*x*(3.f*x-5.f)+2.f;
	float w2 = x*((4.f-3.f*x)*x+1.f);
	float w3 = (x-1.f)*x*x;
	int pn1 = Max( index-1, 0 );
	int pp0 = index;
	int pp1 = Min( index+1, (numPnts-1) );
	int pp2 = Min( index+2, (numPnts-1) );
	const float *pnt0 = &pnts[pn1*numAttr];
	const float *pnt1 = &pnts[pp0*numAttr];
	const float *pnt2 = &pnts[pp1*numAttr];
	const float *pnt3 = &pnts[pp2*numAttr];
	for (int i=0; i<numAttr; i++)
		o[i] = 0.5f*(w0*pnt0[i] + w1*pnt1[i] + w2*pnt2[i] + w3*pnt3[i]);
}

void CatmullRom( float *o, const float *cp, int dimX, int dimY, int numAttr, float u, float v )
{
	float *work = (float*)alloca( dimY * numAttr * sizeof(float) );
	for (int i=0; i<dimY; i++)
	{
		CatmullRom( &work[i*numAttr], &cp[i*dimX*numAttr], numAttr, dimX, u );
	}
	CatmullRom( o, work, numAttr, dimY, v );
}

void GenGrid( float *o, 
	const float *cp0, int dimX0, int dimY0,
	const float *cp1, int dimX1, int dimY1,
	int numAttr, int gx, int gy, float dt )
{
	float *work = (float*)alloca( 2*numAttr*sizeof(float) );
	float invdt = 1.f - dt;
	for (int y=0; y<gy; y++)
	{
		float v = y/(float)(gy-1);
		for (int x=0; x<gx; x++)
		{
			float u = x/(float)(gx-1);
			int index = (y*gx+x)*numAttr;
			float *curo = &o[index];
			CatmullRom( &work[0], cp0, dimX0, dimY0, numAttr, u, v );
			CatmullRom( &work[numAttr], cp1, dimX1, dimY1, numAttr, u, v );
			for (int a=0; a<numAttr; a++)
				curo[a] = invdt*work[a] + dt*work[numAttr+a];
		}
	}
}

void DisplaySkin( SpriterSpacial const *entTM, SpriterSpacial *world, SpriterAnimation &anim, SpriterObject &spriter, float t, TexturePacker const &texture, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor, float tw, float th )
{
	float ms = fmodf( t * 1000.f, anim.length );
	unsigned int mainlineIndex = 0;
	while ( (mainlineIndex+1) < anim.mainlineKeys.size() && ms > anim.mainlineKeys[mainlineIndex+1].time )
		mainlineIndex++;
	SpriterMainlineKey &mlkey = anim.mainlineKeys[mainlineIndex];

	for (unsigned int i=0; i<mlkey.objectRefs.size(); i++)
	{
		SpriterRef &ref = mlkey.objectRefs[i];
		int file = -1;
		int folder = -1;
		float pivot[2] = { FLT_MAX, FLT_MAX };
		SpriterSpacial sample;
		memset( &sample, 0, sizeof(sample));
		if ( ref.timeline >= 0 )
		{
			SpriterTimeline &timeline = anim.timelines[ref.timeline];
			file = timeline.keys[0].file;
			folder = timeline.keys[0].folder;
			sample = timeline.keys[0].spacial;
			pivot[0] = timeline.keys[0].pivot[0];
			pivot[1] = timeline.keys[0].pivot[1];
			for (unsigned int j=1; j<timeline.keys.size(); j++)
			{
				SpriterTimelineKey &pk = timeline.keys[j-1];
				SpriterTimelineKey &ck = timeline.keys[j];
				if ( ms >= pk.time && ms < ck.time )
				{
					file = pk.file;
					folder = pk.folder;
					pivot[0] = pk.pivot[0];
					pivot[1] = pk.pivot[1];
					float dt = (ms - pk.time) / (ck.time-pk.time);
					lerp( sample, pk.spacial, ck.spacial, dt );
					break;
				}
			}
		}

		int skinDimX = -1;
		int skinDimY = -1;
		std::vector<SpriterSkinPnt> skinpnts;
		if ( ref.skin >= 0 )
		{
			SpriterTimeline &timeline = anim.timelines[ref.skin];
			file = timeline.keys[0].file;
			folder = timeline.keys[0].folder;
			sample = timeline.keys[0].spacial;
			pivot[0] = timeline.keys[0].pivot[0];
			pivot[1] = timeline.keys[0].pivot[1];
			for (unsigned int j=1; j<timeline.keys.size(); j++)
			{
				SpriterTimelineKey &pk = timeline.keys[j-1];
				SpriterTimelineKey &ck = timeline.keys[j];
				if ( ms >= pk.time && ms < ck.time )
				{
					SpriterSkin const &psk = pk.skin[ref.s];
					SpriterSkin const &csk = ck.skin[ref.s];
					pivot[0] = pk.pivot[0];
					pivot[1] = pk.pivot[1];
					//if ( psk.dim_x == csk.dim_x && psk.dim_y == csk.dim_y )
					{
						skinDimX = 8;//psk.dim_x;
						skinDimY = 8;//psk.dim_y;
						skinpnts.resize( skinDimX*skinDimY );
						float dt = (ms - pk.time) / (ck.time-pk.time);
						GenGrid( &skinpnts[0].x, 
							&psk.skinpnts[0].x, psk.dim_x, psk.dim_y,
							&csk.skinpnts[0].x, csk.dim_x, csk.dim_y,
							4, skinDimX, skinDimY, dt );
					}
					break;
				}
			}
		}

		if ( file == -1 || folder == -1 )
			continue;

		SpriterFile const &fileInfo = spriter.folders[folder].files[file];
		std::map<std::string,TexturePackerFrame>::const_iterator tf = texture.frames.find(fileInfo.name);
		if ( tf == texture.frames.end() )
			continue;

		pivot[0] = pivot[0] == FLT_MAX ? fileInfo.pivotX : pivot[0];
		pivot[1] = pivot[1] == FLT_MAX ? fileInfo.pivotY : pivot[1];

		SpriterSpacial finalTM;
		if ( ref.parent != -1 )
		{
			SpriterSpacial const &tm = world[ref.parent];
			SpriterSpacial temp;
			mul( temp, sample, tm );
			mul( finalTM, temp, *entTM );
		}
		else
		{
			mul( finalTM, sample, *entTM );
		}

		TexturePackerFrame const &f = tf->second;

		if ( skinpnts.size() )
		{
			float s = sinf(finalTM.angle);
			float c = cosf(finalTM.angle);
			float uo = (f.frame.x) / tw;
			float um = (f.frame.w-1.f) / tw;
			float vm = (f.frame.h-1.f) / th;
			float vo = (f.frame.y) / th;

			for (unsigned int i=0; i<skinpnts.size(); i++)
			{
				float x = /*finalTM.sX */ skinpnts[i].x;
				float y = /*finalTM.sY */ skinpnts[i].y;
				skinpnts[i].x = finalTM.sX * ((x * c) - (y * s)) + finalTM.x; 
				skinpnts[i].y = finalTM.sY * ((x * s) + (y * c)) + finalTM.y;
				skinpnts[i].u = skinpnts[i].u * um + uo;
				skinpnts[i].v = skinpnts[i].v * vm + vo;
			}
			bgfx::setTexture(0, u_texColor, textureColor);
			renderScreenSpaceGrid(1,raymarching, &skinpnts[0].x, skinDimX, skinDimY,
				0xffffffff );
		}
		else
		{
			float ssw = (f.sourceSize.w-1.f);
			float ssh = (f.sourceSize.h-1.f);
			float fw = (fileInfo.width-1.f);
			float fh = (fileInfo.height-1.f);
			float left_x = /*finalTM.sX*/((f.spriteSourceSize.x / ssw)-pivot[0]/*fileInfo.pivotX*/)*fw;
			float top_y = /*finalTM.sY*/((f.spriteSourceSize.y / ssh)-pivot[1]/*fileInfo.pivotY*/)*fh;
			float right_x = /*finalTM.sX*/(((f.spriteSourceSize.x+f.spriteSourceSize.w-1.f) / ssw)-pivot[0]/*fileInfo.pivotX*/)*fw;
			float bottom_y = /*finalTM.sY*/(((f.spriteSourceSize.y+f.spriteSourceSize.h-1.f) / ssh)-pivot[1]/*fileInfo.pivotY*/)*fh;
			float pnts[4][2];
			float s = sinf(finalTM.angle);
			float c = cosf(finalTM.angle);
			pnts[0][0] = finalTM.sX*((left_x * c) - (top_y * s)) + finalTM.x; pnts[0][1] = finalTM.sY*((left_x * s) + (top_y * c)) + finalTM.y;
			pnts[1][0] = finalTM.sX*((right_x * c) - (top_y * s)) + finalTM.x; pnts[1][1] = finalTM.sY*((right_x * s) + (top_y * c)) + finalTM.y;
			pnts[2][0] = finalTM.sX*((right_x * c) - (bottom_y * s)) + finalTM.x; pnts[2][1] = finalTM.sY*((right_x * s) + (bottom_y * c)) + finalTM.y;
			pnts[3][0] = finalTM.sX*((left_x * c) - (bottom_y * s)) + finalTM.x; pnts[3][1] = finalTM.sY*((left_x * s) + (bottom_y * c)) + finalTM.y;

			bgfx::setTexture(0, u_texColor, textureColor);
			renderScreenSpaceQuad(1,raymarching, pnts,
				(f.frame.x) / tw, (f.frame.y+f.frame.h-1.f) / th,
				(f.frame.x+f.frame.w-1.f) / tw, (f.frame.y) / th,
				0xffffffff );
		}
	}
}

void DisplaySkin( SpriterSpacial const *tm, SpriterSpacial *world, SpriterEntity &ent, SpriterObject &spriter, std::string const &animName, float t, TexturePacker const &texture, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor, float tw, float th )
{
	for (unsigned int i=0; i<ent.animations.size(); i++)
	{
		if ( ent.animations[i].name != animName )
			continue;

		DisplaySkin( tm, world, ent.animations[i], spriter, t, texture, raymarching, u_texColor, textureColor, tw, th );
		break;
	}
}

void DisplaySkin( SpriterSpacial const *tm, SpriterSpacial *world, SpriterObject &spriter, std::string const &entityName, std::string const &animName, float t, TexturePacker const &texture, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor, float tw, float th )
{
	for (unsigned int i=0; i<spriter.entities.size(); i++)
	{
		if ( spriter.entities[i].name != entityName )
			continue;
		DisplaySkin( tm, world, spriter.entities[i], spriter, animName, t, texture, raymarching, u_texColor, textureColor, tw, th );
		break;
	}
}
#endif

template <class T>
T* Serialise( const char *filename )
{
	static std::map<std::string,T*> cache;
	std::map<std::string,T*>::iterator f = cache.find( filename );
	if ( f != cache.end() )
	{
		return f->second;
	}

	T* s = new T;
	int len;
	char *start = readTextFile( filename, len );
	if ( start == NULL )
		return false;

	Json::Reader reader;
	Json::Value root;
	if ( !reader.parse( start, root ) )
	{
		free( start );
		return NULL;
	}

	free( start );

	Serialise( root, *s );
	cache[filename] = s;
	return s;
}

template <class T>
bool Serialise( T &s, const char *txt )
{
	Json::Reader reader;
	Json::Value root;
	if ( !reader.parse( txt, root ) )
	{
		return false;
	}

	Serialise( root, s );
	return true;
}

struct SpriterSceneState
{
	SpriterSpacial spacial;
	bgfx::ProgramHandle raymarching;
	bgfx::UniformHandle u_texColor;
};

class SpriterSceneNode
{
	std::vector<SpriterSceneNode*> kids;
	SpriterSpacial spacial;
public:
	SpriterSceneNode()
	{
		spacial.angle = 0.f;
		spacial.sX = 1.f;
		spacial.sY = 1.f;
		spacial.x = 0.f;
		spacial.y = 0.f;
	}

	virtual ~SpriterSceneNode() {}

	void RemoveChild( SpriterSceneNode *node )
	{
		std::vector<SpriterSceneNode*>::iterator f = std::find( kids.begin(), kids.end(), node );
		if ( f != kids.end() )
			kids.erase( f );
	}

	void AddChild( SpriterSceneNode *node )
	{
		kids.push_back( node );
	}

	void AddChildFront( SpriterSceneNode *node )
	{
		kids.insert(kids.begin(), node );
	}

	virtual void RenderInternal(const SpriterSceneState &/*state*/) {}

	virtual void Render( const SpriterSceneState &state )
	{
		SpriterSceneState curstate = state;
		mul( curstate.spacial, spacial, state.spacial );
		RenderInternal( curstate );
		for (unsigned int i=0; i<kids.size(); i++)
		{
			kids[i]->Render( curstate );
		}
	}

	SpriterSpacial &GetSpacial() { return spacial; }
	
	int GetNumKids() { return kids.size(); }
	SpriterSceneNode *GetKid( int index ) { return kids[index]; }
};


void* stdAlloc(void* userData, unsigned int size)
{
	(void)userData;
	return malloc(size);
}

void stdFree(void* userData, void* ptr)
{
	(void)userData;
	free(ptr);
}

class SpriterSceneSprite : public SpriterSceneNode
{
	bgfx::TextureInfo ti;
	bgfx::TextureHandle textureColor;
	float left_x, right_x;
	float top_y, bottom_y;
	float offset_u, offset_v;
public:

	SpriterSceneSprite( const char *filename )
	{
		textureColor = LoadTexture(filename, ti);
		left_x = -ti.width / 2.f;
		right_x =  ti.width / 2.f;
		top_y = -ti.height / 2.f;
		bottom_y = ti.height / 2.f;
		offset_u = 0.f;
		offset_v = 0.f;
	}

	SpriterSceneSprite( const char *filename, float lo, float to, float mw=1.f, float mh=1.f )
	{
		textureColor = LoadTexture(filename, ti);
		left_x = lo;
		right_x =  lo + ti.width * mw;
		top_y = to;
		bottom_y = to + ti.height * mh;
		offset_u = 0.f;
		offset_v = 0.f;
	}

	virtual void RenderInternal( const SpriterSceneState &state )
	{
		float pnts[4][2];
		float s = sin(state.spacial.angle);
        float c = cos(state.spacial.angle);
		pnts[0][0] = state.spacial.sX*((left_x * c) - (top_y * s)) + state.spacial.x; pnts[0][1] = state.spacial.sY*((left_x * s) + (top_y * c)) + state.spacial.y;
		pnts[1][0] = state.spacial.sX*((right_x * c) - (top_y * s)) + state.spacial.x; pnts[1][1] = state.spacial.sY*((right_x * s) + (top_y * c)) + state.spacial.y;
		pnts[2][0] = state.spacial.sX*((right_x * c) - (bottom_y * s)) + state.spacial.x; pnts[2][1] = state.spacial.sY*((right_x * s) + (bottom_y * c)) + state.spacial.y;
		pnts[3][0] = state.spacial.sX*((left_x * c) - (bottom_y * s)) + state.spacial.x; pnts[3][1] = state.spacial.sY*((left_x * s) + (bottom_y * c)) + state.spacial.y;

		bgfx::setTexture(0, state.u_texColor, textureColor);
		renderScreenSpaceQuad(1,state.raymarching, pnts,
			offset_u + 0.f, offset_v + 1.f,
			offset_u + 1.f, offset_v + 0.f,
			0xffffffff );
	}

	inline float GetOffsetU() const { return offset_u; };
	void SetOffsetU( float u ) { offset_u = u; };
};

struct Anim
{
	int  rateMS;
	bool loop;
	std::string name;
	std::vector<int> frames;
};

void Serialise( Json::Value const &node, Anim &anim )
{
	Serialise( node["name"], anim.name );
	Serialise( node["loop"], anim.loop );
	Serialise( node["rateMS"], anim.rateMS );
	Serialise( node["frames"], anim.frames );
}

class SpriterSceneSpriteSheet : public SpriterSceneNode
{
	int tileWidth;
	int tileHeight;
	std::vector<Anim> anims;
	bgfx::TextureInfo ti;
	bgfx::TextureHandle textureColor;
public:

	SpriterSceneSpriteSheet( const char *filename, int tw, int th ) : tileWidth(tw), tileHeight(th)
	{
		textureColor = LoadTexture(filename, ti);
	}

	void AddAnim( std::string const &desc )
	{
		anims.push_back( Anim() );
		Serialise( anims.back(), desc.c_str() );
	}

	virtual void RenderInternal( const SpriterSceneState &state )
	{
		float pnts[4][2];
		float s = sin(state.spacial.angle);
        float c = cos(state.spacial.angle);
		float left_x = -0.5f * tileWidth;
		float right_x = 0.5f * tileWidth;
		float top_y = -0.5f * tileHeight;
		float bottom_y = 0.5f * tileHeight;
		int frame = 2;
		int numX = ti.width / tileWidth;
		float deltaU = (float)tileWidth / ti.width;
		float deltaV = (float)tileHeight / ti.height;
		float offset_u = (frame % numX) * deltaU;
		float offset_v = (frame / numX) * deltaV;
		pnts[0][0] = state.spacial.sX*((left_x * c) - (top_y * s)) + state.spacial.x; pnts[0][1] = state.spacial.sY*((left_x * s) + (top_y * c)) + state.spacial.y;
		pnts[1][0] = state.spacial.sX*((right_x * c) - (top_y * s)) + state.spacial.x; pnts[1][1] = state.spacial.sY*((right_x * s) + (top_y * c)) + state.spacial.y;
		pnts[2][0] = state.spacial.sX*((right_x * c) - (bottom_y * s)) + state.spacial.x; pnts[2][1] = state.spacial.sY*((right_x * s) + (bottom_y * c)) + state.spacial.y;
		pnts[3][0] = state.spacial.sX*((left_x * c) - (bottom_y * s)) + state.spacial.x; pnts[3][1] = state.spacial.sY*((left_x * s) + (bottom_y * c)) + state.spacial.y;

		bgfx::setTexture(0, state.u_texColor, textureColor);
		renderScreenSpaceQuad(1,state.raymarching, pnts,
			offset_u + 0.f, offset_v + deltaU,
			offset_u + deltaV, offset_v + 0.f,
			0xffffffff );
	}
};

class SpriterSceneMesh : public SpriterSceneNode
{
	bgfx::TextureInfo ti;
	bgfx::TextureHandle textureColor;
	std::vector<float> verts;
	std::vector<unsigned short> indices;
public:

	SpriterSceneMesh( const char *filename, float *v, int numV, unsigned short *i, int numI )
	{
		textureColor = LoadTexture(filename, ti);
		verts.resize( numV * 4 );
		indices.resize( numI );
		memcpy( &verts[0], v, numV*4*sizeof(float) );
		memcpy( &indices[0], i, numI*sizeof(unsigned short) );
	}

	virtual void RenderInternal( const SpriterSceneState &state )
	{
		(void)state;
		bgfx::setTexture(0, state.u_texColor, textureColor);
		renderScreenSpaceTrisUV( 1, state.raymarching, &verts[0], &indices[0], 0xffffffff, verts.size()/4, indices.size() );
	}
};


class SpriterSceneSVG : public SpriterSceneNode
{
	std::vector<float> verts;
	std::vector<unsigned short> indices;
public:

	SpriterSceneSVG( const char *filename )
	{
		const bgfx::Memory* mem;
		mem = load( filename );
		struct NSVGImage* svg = nsvgParse((char*)mem->data);
		(void)svg;

		TESSalloc ma;
		TESStesselator* tess = 0;
		memset(&ma, 0, sizeof(ma));
		ma.memalloc = stdAlloc;
		ma.memfree = stdFree;
		ma.userData = NULL;//(void*)&allocated;
		ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

		tess = tessNewTess(&ma);

		for ( struct NSVGShape *shape = svg->shapes; shape; shape=shape->next )
		{
			if ( strcmp( shape->id, "level" ) != 0 )
				continue;
			for (struct NSVGPath*path=shape->paths;path;path=path->next)
			{
				tessAddContour(tess, 2, path->pts, sizeof(float)*2, path->npts);
			}
		}
		// Add contours.
		const int nvp = 3;
		tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_POLYGONS, nvp, 2, 0);

		const int tnverts = tessGetVertexCount(tess);
		const float* tverts = tessGetVertices(tess);
		const int* telems = tessGetElements(tess);
		const int tnelems = tessGetElementCount(tess);
		verts.resize(tnverts*2);
		memcpy( &verts[0], tverts, tnverts*sizeof(float)*2 );
		indices.resize(tnelems*3);
		for (unsigned int i=0; i<indices.size(); i++)
			indices[i] = telems[i];

		if (tess) tessDeleteTess(tess);

	}

	virtual void RenderInternal( const SpriterSceneState &state )
	{
		(void)state;
		renderScreenSpaceTris( 1, state.raymarching, &verts[0], &indices[0], 0xffffffff, verts.size()/2, indices.size() );
	}
};

class SpriterSceneSpriter : public SpriterSceneNode
{
	SpriterObject *spriter;
	TexturePacker *texture;
	bgfx::TextureInfo ti;
	bgfx::TextureHandle textureColor;
	std::vector<SpriterSpacial> local;
	std::vector<SpriterSpacial> world;

	float time;
	std::string anim;
	std::string skin;
public:

	SpriterSceneSpriter( const char *spriterfile, const char *texturepack, const char *texturefile )
	{
		spriter = Serialise<SpriterObject>( spriterfile );
		skin = spriter->entities[0].name;
		anim = spriter->entities[0].animations[0].name;
		texture = Serialise<TexturePacker>( texturepack );
		textureColor = LoadTexture(texturefile, ti);

		local.resize( spriter->maxBones );
		world.resize( spriter->maxBones );
		time = 0.f;
	}

	void SetAnim( std::string const &a ) { if ( a != anim ) { time = 0.f; anim = a; } }
	void SetSkin( std::string const &s ) { skin = s; }
	void SetTime( float t ) { time = t; }

	bool Update( float dt )
	{
		time += dt;
		local.resize(1);
		bool complete = SampleAnimation( &local[0], *spriter, skin.c_str(), anim.c_str(), time );
		local[0].x = 0.f;
		LocalToWorld( &world[0], &local[0], *spriter, skin.c_str(), anim.c_str(), time );
		return complete;
	}

	virtual void RenderInternal( const SpriterSceneState &state )
	{
		DisplaySkin( &state.spacial, &world[0], *spriter, skin.c_str(), anim.c_str(), time, *texture, state.raymarching, state.u_texColor, textureColor, ti.width-1.f, ti.height-1.f );
	}
};

const float PixelsToMeters = 1.f / 100.f;

b2Body* CreateBox2DCollider( b2World &world, void *ent, const char *filename, const char *filter )
{
	b2BodyDef groundBodyDef;
	b2Body* groundBody = world.CreateBody(&groundBodyDef);
	groundBody->SetUserData( ent );

	struct NSVGImage* svg = LoadSVG(filename);

	TESSalloc ma;
	TESStesselator* tess = 0;
	memset(&ma, 0, sizeof(ma));
	ma.memalloc = stdAlloc;
	ma.memfree = stdFree;
	ma.userData = NULL;//(void*)&allocated;
	ma.extraVertices = 256; // realloc not provided, allow 256 extra vertices.

	tess = tessNewTess(&ma);

	for ( struct NSVGShape *shape = svg->shapes; shape; shape=shape->next )
	{
		if ( filter && strstr(shape->label,filter)==NULL )
			continue;
		for (struct NSVGPath*path=shape->paths;path;path=path->next)
		{
			tessAddContour(tess, 2, path->pts, sizeof(float)*2, path->npts);
		}
	}
	// Add contours.
	tessTesselate(tess, TESS_WINDING_POSITIVE, TESS_BOUNDARY_CONTOURS, 0, 0, 0);

	const float* tverts = tessGetVertices(tess);
	const TESSindex* telems = tessGetElements(tess);
	const int tnelems = tessGetElementCount(tess);

    for (int i = 0; i < tnelems; i++) 
	{
		const TESSindex base = telems[i * 2];
		const TESSindex count = telems[i * 2 + 1];
		for (int j = 1; j <= count; j++) 
		{
			const float *v0 = &tverts[(base+j-1) * 2];
			const float *v1 = &tverts[(base+(j-0)%count) * 2];
			// Define the ground box shape.
			b2EdgeShape groundShape;
			groundShape.Set( b2Vec2(v0[0]*PixelsToMeters,v0[1]*PixelsToMeters), b2Vec2(v1[0]*PixelsToMeters,v1[1]*PixelsToMeters) );

			// Add the ground fixture to the ground body.
			b2Fixture *groundFix = groundBody->CreateFixture(&groundShape, 0.0f);
			groundFix->SetUserData( "ground" );
		}
	}

	if (tess) tessDeleteTess(tess);

	return groundBody;
}

void CreateScene( SpriterSceneNode *group, const char *filename, const char *filter )
{
	struct NSVGImage* svg = LoadSVG(filename);

	for ( struct NSVGShape *shape = svg->shapes; shape; shape=shape->next )
	{
		if ( filter && strstr(shape->label,filter)==NULL )
			continue;
		if ( strcmp( shape->type,"image" ) != 0 )
			continue;

		SpriterSceneSprite *sprite = new SpriterSceneSprite(shape->xlink_href);

		sprite->GetSpacial().x = shape->xform[0]*(shape->ofs[0] + shape->dim[0]/2.f);
		sprite->GetSpacial().y = shape->xform[3]*(shape->ofs[1] + shape->dim[1]/2.f);
		sprite->GetSpacial().sX = shape->xform[0];
		sprite->GetSpacial().sY = shape->xform[3];
		group->AddChildFront(sprite);
	}
}

bool FindEntity( float pos[2], const char *filename, const char *filter, const char *id )
{
	struct NSVGImage* svg = LoadSVG(filename);

	for ( struct NSVGShape *shape = svg->shapes; shape; shape=shape->next )
	{
		if ( filter && strstr(shape->label,filter)==NULL )
			continue;
		if ( strcmp( shape->type,"rect" ) != 0 )
			continue;
		if ( id && strcmp(shape->id,id)!=0 )
			continue;
		pos[0] = shape->ofs[0];
		pos[1] = shape->ofs[1];
		return true;
	}
	return false;
}


class Box2DDebugDraw : public b2Draw
{
	bgfx::ProgramHandle program;
public:

	Box2DDebugDraw( bgfx::ProgramHandle _program ) : program(_program)
	{
		SetFlags( GetFlags() | e_shapeBit );
	}

	virtual void DrawPolygon(const b2Vec2* , int32 , const b2Color& ) {}
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& /*color*/) 
	{
		std::vector<float> scaledVerts( vertexCount * 2 );
		for (int i=0; i<vertexCount; i++)
		{
			scaledVerts[i*2+0] = vertices[i].x / PixelsToMeters;
			scaledVerts[i*2+1] = vertices[i].y / PixelsToMeters;
		}
		renderScreenSpaceTris( 1, program, &scaledVerts[0], 0xffffffff, vertexCount );
	}
	virtual void DrawCircle(const b2Vec2&, float32, const b2Color& ) {}
	virtual void DrawSolidCircle(const b2Vec2& c, float32 r, const b2Vec2& , const b2Color& ) 
	{
		int vertexCount = 8;
		std::vector<float> scaledVerts( vertexCount * 2 );
		float scaledR = r / PixelsToMeters;
		for (int i=0; i<vertexCount; i++)
		{
			scaledVerts[i*2+0] = c.x / PixelsToMeters + sinf( (i*2.f*3.14159f) / vertexCount ) *scaledR;
			scaledVerts[i*2+1] = c.y / PixelsToMeters + cosf( (i*2.f*3.14159f) / vertexCount ) *scaledR;
		}
		renderScreenSpaceTris( 1, program, &scaledVerts[0], 0xffffffff, vertexCount );
	}

	virtual void DrawSegment(const b2Vec2& v0, const b2Vec2& v1, const b2Color& ) 
	{
		float scaledVerts[4];
		scaledVerts[0*2+0] = v0.x / PixelsToMeters;
		scaledVerts[0*2+1] = v0.y / PixelsToMeters;
		scaledVerts[1*2+0] = v1.x / PixelsToMeters;
		scaledVerts[1*2+1] = v1.y / PixelsToMeters;
		
		renderScreenSpaceLines( 1, program, &scaledVerts[0], 0xffffffff, 2 );
	}

	virtual void DrawTransform(const b2Transform& ) {}

};

#if 0
struct Sprite
{
	float min_uv[2];
	float max_uv[2];
	int dim[2];
};

struct SubTexture
{
	std::string name;
	float x, y;
	float width, height;

	float min_uv[2];
	float max_uv[2];
};

struct Texture
{
	std::string imagePath;
	std::string name;
	std::vector<SubTexture> SubTextures;
};

void Serialise( Json::Value const &node, SubTexture &subtexture, Texture &/*texture*/ )
{
	Serialise( node["name"], subtexture.name );
	Serialise( node["x"], subtexture.x );
	Serialise( node["y"], subtexture.y );
	Serialise( node["width"], subtexture.width );
	Serialise( node["height"], subtexture.height );
}

void Serialise( Json::Value const &node, Texture &texture )
{
	Serialise( node["imagePath"], texture.imagePath );
	Serialise( node["name"], texture.name );
	Serialise( node["SubTexture"], texture.SubTextures, texture );
}


struct Transform
{
	float skewX, skewY;
	float scaleX, scaleY;
	float x, y;
	float pX, pY;
};

void Serialise( Json::Value const &node, Transform &transform )
{
	Serialise( node["skX"], transform.skewX );
	Serialise( node["skY"], transform.skewY );
	Serialise( node["scX"], transform.scaleX );
	Serialise( node["scY"], transform.scaleY );
	Serialise( node["x"], transform.x );
	Serialise( node["y"], transform.y );
	Serialise( node["pX"], transform.pX );
	Serialise( node["pY"], transform.pY );
}

struct ColourTransform
{
	float aO, rO, gO, bO;
	float aM, rM, gM, bM;
};

void Serialise( Json::Value const &node, ColourTransform &colourTransform )
{
	Serialise( node["aO"], colourTransform.aO );
	Serialise( node["rO"], colourTransform.rO );
	Serialise( node["gO"], colourTransform.gO );
	Serialise( node["bO"], colourTransform.bO );
	Serialise( node["aM"], colourTransform.aM );
	Serialise( node["rM"], colourTransform.rM );
	Serialise( node["gM"], colourTransform.gM );
	Serialise( node["bM"], colourTransform.bM );
}

struct Matrix2x3
{
	float a, b, c, d;
	float tx, ty;
};

inline void TransformToMatrix( Transform const &t, Matrix2x3 &m )
{
	float radSkewX = DegToRad( t.skewX );
	float radSkewY = DegToRad( t.skewY );
	m.a = t.scaleX * cosf( radSkewY );
	m.b = t.scaleX * sinf( radSkewY );
	m.c = -t.scaleY * sinf( radSkewX );
	m.d = t.scaleY * cosf( radSkewX );
	m.tx = t.x;
	m.ty = t.y;
}

struct Display
{
	Transform transform;
	std::string name;
	std::string type;

	int nameIndex;
};

struct Slot
{
	std::string name;
	float z;
	std::vector<Display> displays;
	std::string parent;

	int parentIndex;
};

struct Skin
{
	std::string name;
	std::vector<Slot> slots;
};

struct Bone
{
	Transform transform;
	std::string name;
	std::string parent;

	int parentIndex;
};

struct Frame
{
	Transform transform;
	float z;
	int hide;
	float tweenEasing;
	float tweenRotate;
	int displayIndex;
	float duration;
	std::string evt;
	std::string sound;
	std::string action;
};

struct Timeline
{
	std::string name;
	float scale;
	std::vector<Frame> frames;
	float offset;

	int nameIndex;
};

struct Animation
{
	std::string name;
	float scale;
	int loop;
	std::vector<Timeline> timeline;
	float fadeInTime;
	float duration;
	float tweenEasing;
};

struct Armature
{
	std::vector<Skin> skins;
	std::vector<Bone> bones;
	std::string name;
	std::vector<Animation> animations;
};

struct Skeleton
{
	int frameRate;
	std::string name;
	float version;
	std::vector<Armature> armature;
};

void Serialise( Json::Value const &node, Display &display, Armature & /*armature*/, Texture &texture )
{
	Serialise( node["transform"], display.transform );
	Serialise( node["name"], display.name );
	Serialise( node["type"], display.type );

	display.nameIndex = -1;
	for (unsigned int i=0; i<texture.SubTextures.size(); i++)
	{
		if ( texture.SubTextures[i].name == display.name)
		{
			display.nameIndex = i;
			break;
		}
	}
}

void Serialise( Json::Value const &node, Slot &slot, Armature &armature, Texture &texture )
{
	Serialise( node["name"], slot.name );
	Serialise( node["z"], slot.z );
	Serialise( node["display"], slot.displays, armature, texture );
	Serialise( node["parent"], slot.parent );

	slot.parentIndex = -1;
	for (unsigned int i=0; i<armature.bones.size(); i++)
	{
		if ( armature.bones[i].name == slot.parent )
		{
			slot.parentIndex = i;
			break;
		}
	}
}

void Serialise( Json::Value const &node, Skin &skin, Armature &armature, Texture &texture )
{
	Serialise( node["name"], skin.name );
	Serialise( node["slot"], skin.slots, armature, texture );
}

void Serialise( Json::Value const &node, Bone &bone, Armature &armature )
{
	Serialise( node["transform"], bone.transform );
	Serialise( node["name"], bone.name );
	Serialise( node["parent"], bone.parent );

	bone.parentIndex = -1;
	for (unsigned int i=0; i<armature.bones.size(); i++)
	{
		if ( armature.bones[i].name == bone.parent )
		{
			bone.parentIndex = i;
			break;
		}
	}
}

void Serialise( Json::Value const &node, Frame &frame, Armature & /*armature*/ )
{
	Serialise( node["transform"], frame.transform );
	Serialise( node["duration"], frame.duration  );
	Serialise( node["event"], frame.evt  );
	Serialise( node["sound"], frame.sound  );
	Serialise( node["action"], frame.action  );
	Serialise( node["hide"], frame.hide );
	Serialise( node["tweenEasing"], frame.tweenEasing );
	Serialise( node["tweenRotate"], frame.tweenRotate );
	Serialise( node["displayIndex"], frame.displayIndex );
	Serialise( node["z"], frame.z );
}

void Serialise( Json::Value const &node, Timeline &timeline, Armature &armature )
{
	Serialise( node["name"], timeline.name );
	Serialise( node["scale"], timeline.scale );
	Serialise( node["frame"], timeline.frames, armature );
	Serialise( node["offset"], timeline.offset );

	timeline.nameIndex = -1;
	for (unsigned int i=0; i<armature.bones.size(); i++)
	{
		if ( armature.bones[i].name == timeline.name )
		{
			timeline.nameIndex = i;
			break;
		}
	}
}

void CalcTimeline( std::vector<Transform> &transforms, Timeline &timeline, float time )
{
	Transform &transform = transforms[timeline.nameIndex];
	float currentStartTime = 0.f;
	for (unsigned int i=1; i<=timeline.frames.size(); i++)
	{
		unsigned int prev = i - 1;
		Frame const &frame0 = timeline.frames[prev];
		Frame const &frame1 = timeline.frames[(i%timeline.frames.size())];

		float currentEndTime = currentStartTime + frame0.duration;

		if ( time >= currentStartTime && time<currentEndTime )
		{
			float dt = (time - currentStartTime)/(currentEndTime-currentStartTime);
			float invdt = 1.f - dt;
			transform.pX = frame0.transform.pX * dt + frame1.transform.pX * invdt;
			transform.pY = frame0.transform.pY * dt + frame1.transform.pY * invdt;
			transform.scaleX = frame0.transform.scaleX * dt + frame1.transform.scaleX * invdt;
			transform.scaleY = frame0.transform.scaleY * dt + frame1.transform.scaleY * invdt;
			transform.skewX = frame0.transform.skewX * dt + frame1.transform.skewX * invdt;
			transform.skewY = frame0.transform.skewY * dt + frame1.transform.skewY * invdt;
			transform.x = frame0.transform.x * dt + frame1.transform.x * invdt;
			transform.y = frame0.transform.y * dt + frame1.transform.y * invdt;
			break;
		}

		currentStartTime = currentEndTime;
	}
}

void Serialise( Json::Value const &node, Animation &animation, Armature &armature )
{
	Serialise( node["name"], animation.name );
	Serialise( node["scale"], animation.scale );
	Serialise( node["loop"], animation.loop );
	Serialise( node["timeline"], animation.timeline, armature );
	Serialise( node["fadeInTime"], animation.fadeInTime );
	Serialise( node["duration"], animation.duration );
	Serialise( node["tweenEasing"], animation.tweenEasing );
}

void CalcAnimation( std::vector<Transform> &transforms, Animation &animation, float time )
{
	for (unsigned int i=0; i<animation.timeline.size(); i++)
	{
		CalcTimeline( transforms, animation.timeline[i], time );
	}
}

void Serialise( Json::Value const &node, Armature &armature, Skeleton & /*skel*/, Texture &texture )
{
	Serialise( node["name"], armature.name );
	Serialise( node["bone"], armature.bones, armature );

	Serialise( node["skin"], armature.skins, armature, texture );
	Serialise( node["animation"], armature.animations, armature );
}

void DisplaySkin( Texture &texture, Skin &skin, std::vector<Transform> &transforms, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor )
{
	(void)texture;
	(void)transforms;
	for ( unsigned int i=0; i<skin.slots.size(); i++ )
	{
		Slot &slot = skin.slots[i];
		Transform &transform = transforms[slot.parentIndex];

		if ( slot.z > 4 )
			continue;

		for (unsigned int j=0; j<slot.displays.size(); j++)
		{
			Display &display = slot.displays[j];
			SubTexture &subtexture = texture.SubTextures[display.nameIndex];
			
			bgfx::setTexture(0, u_texColor, textureColor);
			renderScreenSpaceQuad(1,raymarching, transform.x, transform.y, subtexture.width, subtexture.height, subtexture.min_uv[0], subtexture.min_uv[1], subtexture.max_uv[0], subtexture.max_uv[1], 0xffffffff );
		}
	}
}

void DisplayArmature( Texture &texture, Armature &armature, std::string const &skinname, std::string const &animname, float time, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor )
{
	int skinIndex = -1;
	for (unsigned int s=0; s<armature.skins.size(); s++)
	{
		if ( armature.skins[s].name != skinname )
			continue;
		skinIndex = s;
		break;
	}

	if ( skinIndex == -1 )
		return;

	for (unsigned int a=0; a<armature.animations.size(); a++)
	{
		if ( armature.animations[a].name != animname )
			continue;

		std::vector<Transform> transforms(armature.bones.size());
		CalcAnimation( transforms, armature.animations[a], fmodf( time, armature.animations[a].duration ) );
		DisplaySkin( texture, armature.skins[skinIndex], transforms, raymarching, u_texColor, textureColor );
		return;
	}

}

void Serialise( Json::Value const &node, Skeleton &skel, Texture &texture )
{
	Serialise( node["frameRate"], skel.frameRate );
	Serialise( node["name"], skel.name );
	Serialise( node["version"], skel.version );
	Serialise( node["armature"], skel.armature, skel, texture );
}

void DisplaySkeleton( Texture &texture, Skeleton &skeleton, std::string const &skinname, std::string const &animname, float time, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor )
{
	for ( unsigned int a=0; a<skeleton.armature.size(); a++)
	{
		DisplayArmature( texture, skeleton.armature[a], skinname, animname, time, raymarching, u_texColor, textureColor );
	}
}
#endif


#if 0
struct Bone
{
	std::string name;
	std::string parent;
	float length, x, y, rotation;

	int parentIndex;
};

struct Slot
{
	std::string name;
	std::string bone;
	std::string attachment;

	int boneIndex;
};

struct SkinFrame
{
	float x;
	float y;
	float rotation;
	float width;
	float height;
};

struct SkinEntry
{
	std::vector<std::pair<std::string,SkinFrame>> skinFrames;
};

struct Skin
{
	std::vector<std::pair<std::string,SkinEntry>> skinEntries;
	
	std::vector<int> skinSlotIndex;
};

struct RotateKey
{
	float time;
	float angle;
};

struct TranslateKey
{
	float time;
	float x, y;
};

struct BoneTimeline
{
	std::vector<RotateKey> rotateKeys;
	std::vector<TranslateKey> translateKeys;
};

struct Animation
{
	std::vector<std::pair<std::string,BoneTimeline>> boneTimeline;
	
	std::vector<int> boneMapping;
};

struct Spine
{
	std::vector<Bone> bones;
	std::vector<Slot> slots;
	std::map<std::string,Skin> skins;
	std::map<std::string,Animation> animations;
};

void Serialise( Json::Value const &node, Bone &bone, Spine &spine )
{
	Serialise( node["name"], bone.name );
	Serialise( node["parent"], bone.parent );
	Serialise( node["length"], bone.length );
	Serialise( node["x"], bone.x );
	Serialise( node["y"], bone.y );
	Serialise( node["rotation"], bone.rotation );

	bone.parentIndex = -1;
	for (unsigned int i=0; i<spine.bones.size(); i++)
	{
		if ( bone.parent == spine.bones[i].name )
		{
			bone.parentIndex = i;
			break;
		}
	}
}

void Serialise( Json::Value const &node, Slot &slot, Spine &spine )
{
	Serialise( node["name"], slot.name );
	Serialise( node["bone"], slot.bone );
	Serialise( node["attachment"], slot.attachment );

	slot.boneIndex = -1;
	for (unsigned int i=0; i<spine.bones.size(); i++)
	{
		if ( slot.bone == spine.bones[i].name )
		{
			slot.boneIndex = i;
			break;
		}
	}
}

void Serialise( Json::Value const &node, SkinFrame &skinFrame )
{
	Serialise( node["x"], skinFrame.x );
	Serialise( node["y"],  skinFrame.y );
	Serialise( node["rotation"], skinFrame.rotation );
	Serialise( node["width"], skinFrame.width );
	Serialise( node["height"], skinFrame.height );
}

void Serialise( Json::Value const &node, SkinEntry &skinEntry )
{
	Serialise( node, skinEntry.skinFrames );
}

void Serialise( Json::Value const &node, Skin &skin, Spine &spine )
{
	Serialise( node, skin.skinEntries );

	skin.skinSlotIndex.resize( skin.skinEntries.size(), -1 );
	for (unsigned int i=0; i<skin.skinEntries.size(); i++)
	{
		for (unsigned int j=0; j<spine.slots.size(); j++)
		{
			if ( skin.skinEntries[i].first == spine.slots[j].name )
			{
				skin.skinSlotIndex[i] = j;
				break;
			}
		}
	}
}

void Serialise( Json::Value const &node, RotateKey &rotateKey )
{
	Serialise( node["time"], rotateKey.time );
	Serialise( node["angle"], rotateKey.angle );
}

void Serialise( Json::Value const &node, TranslateKey &translateKey )
{
	Serialise( node["time"], translateKey.time );
	Serialise( node["x"], translateKey.x );
	Serialise( node["y"], translateKey.y );
}

void Serialise( Json::Value const &node, BoneTimeline &boneTimeline )
{
	Serialise( node["rotate"], boneTimeline.rotateKeys );
	Serialise( node["translate"], boneTimeline.translateKeys );
}

void Serialise( Json::Value const &node, Animation &animation, Spine &spine )
{
	Serialise( node["bones"], animation.boneTimeline );

	animation.boneMapping.resize(animation.boneTimeline.size(),-1);
	for (unsigned int i=0; i<animation.boneTimeline.size(); i++)
	{
		for (unsigned int j=0; j<spine.bones.size(); j++)
		{
			if ( animation.boneTimeline[i].first == spine.bones[j].name )
			{
				animation.boneMapping[i] = j;
				break;
			}
		}
	}
}

void Serialise( Json::Value const &node, Spine &spine )
{
	Serialise( node["bones"], spine.bones, spine );
	Serialise( node["slots"], spine.slots, spine );
	Serialise( node["skins"], spine.skins, spine );
	Serialise( node["animations"], spine.animations, spine );
}

void SampleAnimation( Matrix2x3 *local, Animation const &anim, Spine &spine, float t )
{
	for (unsigned int i=0; i<anim.boneTimeline.size(); i++)
	{
		BoneTimeline const &timeline = anim.boneTimeline[i].second;
		int index = anim.boneMapping[i];
		Matrix2x3 &mtx = local[index];

		float angle = 0.f;
		float x = 0.f;
		float y = 0.f;
		if ( index != -1 )
		{
			Bone &bone = spine.bones[index];
			angle += bone.rotation;
			x += bone.x;
			y += bone.y;
		}
		
		if ( timeline.rotateKeys.size() )
		{
			float rt = fmodf( t, timeline.rotateKeys[timeline.rotateKeys.size()-1].time );
			for (unsigned int j=1; j<timeline.rotateKeys.size(); j++)
			{
				RotateKey const &pk = timeline.rotateKeys[j-1];
				RotateKey const &ck = timeline.rotateKeys[j];
				if ( rt >= pk.time && rt < ck.time )
				{
					float dt = (rt - pk.time) / ( ck.time - pk.time );
					angle += InterpAngle( ck.angle, pk.angle, dt );
					break;
				}
			}
		}
		if ( timeline.translateKeys.size() )
		{
			float tt = fmodf( t, timeline.translateKeys[timeline.translateKeys.size()-1].time );
			for (unsigned int j=1; j<timeline.translateKeys.size(); j++)
			{
				TranslateKey const &pk = timeline.translateKeys[j-1];
				TranslateKey const &ck = timeline.translateKeys[j];
				if ( tt >= pk.time && tt < ck.time )
				{
					float dt = (tt - pk.time) / ( ck.time - pk.time );
					float invdt = 1.f - dt;
					x += invdt * pk.x + dt * ck.x;
					y += invdt * pk.y + dt * ck.y;
					break;
				}
			}
		}
		float radAngle = DegToRad( angle );
		mtx.f[0][0] = cosf(radAngle);
		mtx.f[0][1] = sinf(radAngle);
		mtx.f[1][0] = -sinf(radAngle);
		mtx.f[1][1] = cosf(radAngle);
		mtx.f[2][0] = x;
		mtx.f[2][1] = y;
	}
}

void SampleAnimation( Matrix2x3 *world, Spine &spine, std::string const &animName, float t )
{
	for (unsigned int i=0; i<spine.bones.size(); i++)
	{
		Bone &bone = spine.bones[i];
		Matrix2x3 &mtx = world[i];
		float radAngle = DegToRad( bone.rotation );
		mtx.f[0][0] = cosf(radAngle);
		mtx.f[0][1] = sinf(radAngle);
		mtx.f[1][0] = -sinf(radAngle);
		mtx.f[1][1] = cosf(radAngle);
		mtx.f[2][0] = bone.x;
		mtx.f[2][1] = bone.y;
	}
	std::map<std::string,Animation>::const_iterator f = spine.animations.find(animName);
	if ( f == spine.animations.end() )
		return;

	SampleAnimation( world, f->second, spine, t );
}


void LocalToWorld( Matrix2x3 *world, Matrix2x3 *local, Spine &spine )
{
	for (unsigned int i=0; i<spine.bones.size(); i++)
	{
		Bone const &bone = spine.bones[i];
		if ( bone.parentIndex == -1 )
		{
			world[i] = local[i];
			continue;
		}

		mul( world[i], local[i], world[bone.parentIndex] );
	}
}

void DisplaySkin( Matrix2x3 *world, Spine &spine, std::string const &skinName, TexturePacker const &texture, bgfx::ProgramHandle raymarching, bgfx::UniformHandle u_texColor, bgfx::TextureHandle textureColor, float tw, float th )
{
	std::map<std::string,Skin>::const_iterator f = spine.skins.find(skinName);
	if ( f == spine.skins.end() )
		return;

	Skin const &skin = f->second;
	for (unsigned int i=0; i<skin.skinEntries.size(); i++)
	{
		SkinEntry const &se = skin.skinEntries[i].second;
		if ( se.skinFrames.empty() )
			continue;
		int slotIndex = skin.skinSlotIndex[i];
		if ( slotIndex == -1 )
			continue;
		int mtxIndex = spine.slots[slotIndex].boneIndex;
		if ( mtxIndex == -1 )
			continue;
		Matrix2x3 &tm = world[mtxIndex];
		SkinFrame const &sf = se.skinFrames[0].second;
		
		std::map<std::string,TexturePackerFrame>::const_iterator tf = texture.frames.find(se.skinFrames[0].first);
		if ( tf == texture.frames.end() )
			continue;

		Matrix2x3 spriteTM;
		float radAngle = DegToRad( sf.rotation );
		spriteTM.f[0][0] = cosf(radAngle);
		spriteTM.f[0][1] = sinf(radAngle);
		spriteTM.f[1][0] = -sinf(radAngle);
		spriteTM.f[1][1] = cosf(radAngle);
		spriteTM.f[2][0] = sf.x;
		spriteTM.f[2][1] = sf.y;

		Matrix2x3 finalTM;
		mul( finalTM, spriteTM, tm );

		float hw = sf.width / 2.f;
		float hh = sf.height / 2.f;
		float pnts[4][2];
		mul( pnts[0], -hw, -hh, finalTM );
		mul( pnts[1], hw, -hh, finalTM );
		mul( pnts[2], hw, hh, finalTM );
		mul( pnts[3], -hw, hh, finalTM );

		TexturePackerFrame const &f = tf->second;

		bgfx::setTexture(0, u_texColor, textureColor);
		renderScreenSpaceQuad(1,raymarching, pnts,
			(f.frame.x) / tw, (f.frame.y+f.frame.h) / th,
			(f.frame.x+f.frame.w) / tw, (f.frame.y) / th,
			0xffffffff );
	}
}
#endif



template <class T>
void Serialise( pugi::xml_node node, const char *name, std::vector<T> &val )
{
	pugi::xml_node c = node.child( name );
	while ( c )
	{
		val.push_back( T() );
		T &v = val.back();
		Serialise( c, v );
		c = c.next_sibling( name );
	}
}

template <class T, class P0>
void Serialise( pugi::xml_node node, const char *name, std::vector<T> &val, P0 &p0 )
{
	pugi::xml_node c = node.child( name );
	while ( c )
	{
		val.push_back( T() );
		T &v = val.back();
		Serialise( c, v, p0 );
		c = c.next_sibling( name );
	}
}

template <class T, class Predicate>
void SerialisePredicate( pugi::xml_node node, const char *name, std::vector<T> &val, Predicate const &p )
{
	pugi::xml_node c = node.child( name );
	while ( c )
	{
		if ( p(c) )
		{
			val.push_back( T() );
			T &v = val.back();
			Serialise( c, v );
		}
		c = c.next_sibling( name );
	}
}

void Serialise( pugi::xml_attribute attr, bool &val )
{
	if ( !attr )
	{
		val = false;
		return;
	}
	val = attr.as_bool();
}

void Serialise( pugi::xml_attribute attr, int &val, int def=0 )
{
	if ( !attr )
	{
		val = def;
		return;
	}
	val = attr.as_int();
}

void Serialise( pugi::xml_attribute attr, float &val, float def = 0.f )
{
	if ( !attr )
	{
		val = def;
		return;
	}
	val = attr.as_float();
}

void Serialise( pugi::xml_attribute attr, std::string &val )
{
	if ( !attr )
	{
		val = "";
		return;
	}
	val = attr.as_string();
}


struct TiledImage
{
	std::string source;
	std::string trans;
	int width;
	int height;
};

struct TiledTileset
{
	int firstgid;
	std::string name;
	int tilewidth;
	int tileheight;
	std::vector<TiledImage> images;
};

struct TiledData
{
	std::vector<int> ids;
};

struct TiledLayer
{
	std::string name;
	int width;
	int height;
	std::vector<TiledData> datas;
};

struct TiledObject
{
	std::string name;
	int gid;
	int x,y;
};

struct TiledObjectGroup
{
	std::string name;
	std::vector<TiledObject> objects;
};

struct TiledMap
{
	float version;
	std::string orientation;
	int width;
	int height;
	int tilewidth;
	int tileheight;
	
	std::vector<TiledTileset> tilesets;
	std::vector<TiledLayer> layers;
	std::vector<TiledObjectGroup> objectgroups;
};

void Serialise( pugi::xml_node node, TiledData &data )
{
	const char *c = node.text().as_string();
	while ( *c && *c <= 32 )
		c++;
	int decodeLen = Base64decode_len( c );
	char *decode = (char*)malloc( decodeLen+1 );
	Base64decode( decode, c );
	int out;
	char *uncomp = zlib_decode_malloc( decode, decodeLen, &out );
	data.ids.resize( out / 4 );
	memcpy( &data.ids[0], uncomp, data.ids.size()*4 );
	free( uncomp );
	free( decode );
}

void Serialise( pugi::xml_node node, TiledLayer &layer )
{
	Serialise( node.attribute("name"), layer.name );
	Serialise( node.attribute("width"), layer.width );
	Serialise( node.attribute("height"), layer.height );
	Serialise( node, "data", layer.datas );
}

void Serialise( pugi::xml_node node, TiledImage &image )
{
	Serialise( node.attribute("source"), image.source );
	Serialise( node.attribute("trans"), image.trans );
	Serialise( node.attribute("width"), image.width );
	Serialise( node.attribute("height"), image.height );
}

void Serialise( pugi::xml_node node, TiledTileset &tileset )
{
	Serialise( node.attribute("firstgid"), tileset.firstgid );
	Serialise( node.attribute("name"), tileset.name );
	Serialise( node.attribute("tilewidth"), tileset.tilewidth );
	Serialise( node.attribute("tileheight"), tileset.tileheight );	
	Serialise( node, "image", tileset.images );
}

void Serialise( pugi::xml_node node, TiledObject &object )
{
	Serialise( node.attribute("name"), object.name );
	Serialise( node.attribute("x"), object.x );
	Serialise( node.attribute("y"), object.y );	
	Serialise( node.attribute("gid"), object.gid );	
}

void Serialise( pugi::xml_node node, TiledObjectGroup &objectgroup )
{
	Serialise( node.attribute("name"), objectgroup.name );
	Serialise( node, "object", objectgroup.objects );
}

void Serialise( pugi::xml_node node, TiledMap &map )
{
	Serialise( node.attribute("version"), map.version );
	Serialise( node.attribute("orientation"), map.orientation );
	Serialise( node.attribute("width"), map.width );
	Serialise( node.attribute("height"), map.height );
	Serialise( node.attribute("tilewidth"), map.tilewidth );
	Serialise( node.attribute("tileheight"), map.tileheight );	
	Serialise( node, "tileset", map.tilesets );
	Serialise( node, "layer", map.layers );
	Serialise( node, "objectgroup", map.objectgroups );
}

#if 0
void Serialise( pugi::xml_node node, SpriterFile &spriteFile )
{
	Serialise( node.attribute("name"), spriteFile.name );
	Serialise( node.attribute("pivot_x"), spriteFile.pivotX );
	Serialise( node.attribute("pivot_y"), spriteFile.pivotY );
	Serialise( node.attribute("width"), spriteFile.width );
	Serialise( node.attribute("height"), spriteFile.height );
}

void Serialise( pugi::xml_node node, SpriterFolder &spriteFolder )
{
	Serialise( node.attribute("name"), spriteFolder.name );
	Serialise( node, "file", spriteFolder.files );
}

void Serialise( pugi::xml_node node, SpriterTimelineKey &spriteTimelineKey )
{
	Serialise( node.attribute("time"), spriteTimelineKey.time );
	pugi::xml_node bone = node.child("bone");
	if ( bone )
	{
		Serialise( bone.attribute("x"), spriteTimelineKey.spacial.x );
		Serialise( bone.attribute("y"), spriteTimelineKey.spacial.y );
		Serialise( bone.attribute("scale_x"), spriteTimelineKey.spacial.sX, 1.f );
		Serialise( bone.attribute("scale_y"), spriteTimelineKey.spacial.sY, 1.f );
		Serialise( bone.attribute("angle"), spriteTimelineKey.spacial.angle );
		spriteTimelineKey.file = -1;
		spriteTimelineKey.folder = -1;
	}
	pugi::xml_node object = node.child("object");
	if ( object )
	{
		Serialise( object.attribute("x"), spriteTimelineKey.spacial.x );
		Serialise( object.attribute("y"), spriteTimelineKey.spacial.y );
		Serialise( object.attribute("scale_x"), spriteTimelineKey.spacial.sX, 1.f );
		Serialise( object.attribute("scale_y"), spriteTimelineKey.spacial.sY, 1.f );
		Serialise( object.attribute("angle"), spriteTimelineKey.spacial.angle );
		Serialise( object.attribute("file"), spriteTimelineKey.file );
		Serialise( object.attribute("folder"), spriteTimelineKey.folder );
	}
	spriteTimelineKey.spacial.angle = DegToRad( spriteTimelineKey.spacial.angle );
}

void Serialise( pugi::xml_node node, SpriterTimeline &spriteTimeline, SpriterAnimation &anim )
{
	Serialise( node.attribute("name"), spriteTimeline.name );
	Serialise( node, "key", spriteTimeline.keys );
	spriteTimeline.keys.push_back( spriteTimeline.keys[0] );
	spriteTimeline.keys.back().time = (int)anim.length;
}

class WhereAttributeEquals
{
	std::string attr;
	std::string value;
public:
	WhereAttributeEquals( const char *a, const char *v ) : attr(a), value(v) {}

	inline bool operator()(pugi::xml_node node) const
	{
		return node.attribute(attr.c_str()).as_string() == value;
	}
};

void Serialise( pugi::xml_node node, SpriterRef &ref )
{
	Serialise( node.attribute("parent"), ref.parent, -1 );
	Serialise( node.attribute("timeline"), ref.timeline );
	Serialise( node.attribute("key"), ref.key );
}

void Serialise( pugi::xml_node node, SpriterMainlineKey &spriterMainline, SpriterObject &spriter )
{
	Serialise( node.attribute("time"), spriterMainline.time );
	Serialise( node, "bone_ref", spriterMainline.boneRefs );
	Serialise( node, "object_ref", spriterMainline.objectRefs );
	if ( spriterMainline.boneRefs.size() > spriter.maxBones )
		spriter.maxBones = spriterMainline.boneRefs.size();
}

void Serialise( pugi::xml_node node, SpriterAnimation &spriteAnimation, SpriterObject &spriter )
{
	Serialise( node.attribute("name"), spriteAnimation.name );
	Serialise( node.attribute("length"), spriteAnimation.length );
	Serialise( node.attribute("loopType"), spriteAnimation.looping );
	Serialise( node.child("mainline"), "key", spriteAnimation.mainlineKeys, spriter );
	Serialise( node, "timeline", spriteAnimation.timelines, spriteAnimation );
}

void Serialise( pugi::xml_node node, SpriterObjInfo &spriterObjInfo )
{
	Serialise( node.attribute("name"), spriterObjInfo.name );
	Serialise( node.attribute("w"), spriterObjInfo.w );
	Serialise( node.attribute("h"), spriterObjInfo.h );
}

void Serialise( pugi::xml_node node, SpriterEntity &spriteEntity, SpriterObject &spriter )
{
	Serialise( node.attribute("name"), spriteEntity.name );
	SerialisePredicate( node, "obj_info", spriteEntity.boneObjInfo, WhereAttributeEquals("type","bone") );
	Serialise( node, "animation", spriteEntity.animations, spriter );
}

void Serialise( pugi::xml_node node, SpriterObject &spriter )
{
	spriter.maxBones = 0;
	Serialise( node, "folder", spriter.folders );
	Serialise( node, "entity", spriter.entities, spriter );

	//Serialise( node, "activeCharacterMap", activeCharacterMap );
}
#endif

#if 0
// fun.cpp

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <iostream>
#include <sstream>

class Foo
{
public:
    Foo(const std::string & name) : name(name)
    {
        std::cout << "Foo is born" << std::endl;
    }

    std::string Add(int a, int b)
    {
        std::stringstream ss;
        ss << name << ": " << a << " + " << b << " = " << (a+b);
        return ss.str();
    }

    ~Foo()
    {
        std::cout << "Foo is gone" << std::endl;
    }

private:
    std::string name;
};

// The general pattern to binding C++ class to Lua is to write a Lua 
// thunk for every method for the class, so here we go:

int l_Foo_constructor(lua_State * l)
{
    const char * name = luaL_checkstring(l, 1);

    // We could actually allocate Foo itself as a user data but 
    // since user data can be GC'ed and we gain unity by using CRT's heap 
    // all along.
    Foo ** udata = (Foo **)lua_newuserdata(l, sizeof(Foo *));
    *udata = new Foo(name);

    // Usually, we'll just use "Foo" as the second parameter, but I 
    // say luaL_Foo here to distinguish the difference:
    //
    // This 2nd parameter here is an _internal label_ for luaL, it is 
    // _not_ exposed to Lua by default.
    //
    // Effectively, this metatable is not accessible by Lua by default.
    luaL_getmetatable(l, "luaL_Foo");

    // The Lua stack at this point looks like this:
    //     
    //     3| metatable "luaL_foo"   |-1
    //     2| userdata               |-2
    //     1| string parameter       |-3
    //
    // So the following line sets the metatable for the user data to the luaL_Foo 
    // metatable
    //
    // We must set the metatable here because Lua prohibits setting 
    // the metatable of a userdata in Lua. The only way to set a metatable 
    // of a userdata is to do it in C.
    lua_setmetatable(l, -2);

    // The Lua stack at this point looks like this:
    //     
    //     2| userdata               |-1
    //     1| string parameter       |-2
    // 
    // We return 1 so Lua callsite will get the user data and 
    // Lua will clean the stack after that.

    return 1;
}

Foo * l_CheckFoo(lua_State * l, int n)
{
    // This checks that the argument is a userdata 
    // with the metatable "luaL_Foo"
    return *(Foo **)luaL_checkudata(l, n, "luaL_Foo");
}

int l_Foo_add(lua_State * l)
{
    Foo * foo = l_CheckFoo(l, 1);
    int a = (int)luaL_checknumber(l, 2);
    int b = (int)luaL_checknumber(l, 3);

    std::string s = foo->Add(a, b);
    lua_pushstring(l, s.c_str());

    // The Lua stack at this point looks like this:
    //     
    //     4| result string          |-1
    //     3| metatable "luaL_foo"   |-2
    //     2| userdata               |-3
    //     1| string parameter       |-4
    //
    // Return 1 to return the result string to Lua callsite.

    return 1;
}

int l_Foo_destructor(lua_State * l)
{
    Foo * foo = l_CheckFoo(l, 1);
    delete foo;

    return 0;
}

void luaL_RegisterRequireFunc( lua_State * L, const char *name, lua_CFunction func )
{
	/* add open functions from 'preloadedlibs' into 'package.preload' table */
	luaL_getsubtable(L, LUA_REGISTRYINDEX, "_PRELOAD");
	lua_pushcfunction(L, func);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);  /* remove _PRELOAD table */
}

int lua_foorequire(lua_State *l)
{
	luaL_getmetatable(l, "luaL_Foo");  /* try to get metatable */

	return 1;
}

void RegisterFoo(lua_State * l)
{
    luaL_Reg sFooRegs[] =
    {
        { "new", l_Foo_constructor },
        { "add", l_Foo_add },
        { "__gc", l_Foo_destructor },
        { NULL, NULL }
    };
 
    // Create a luaL metatable. This metatable is not 
    // exposed to Lua. The "luaL_Foo" label is used by luaL
    // internally to identity things.
    luaL_newmetatable(l, "luaL_Foo");
 
    // Register the C functions _into_ the metatable we just created.
    luaL_register(l, NULL, sFooRegs);
 
    // The Lua stack at this point looks like this:
    //     
    //     1| metatable "luaL_Foo"   |-1
    lua_pushvalue(l, -1);
 
    // The Lua stack at this point looks like this:
    //     
    //     2| metatable "luaL_Foo"   |-1
    //     1| metatable "luaL_Foo"   |-2
 
    // Set the "__index" field of the metatable to point to itself
    // This pops the stack
    lua_setfield(l, -1, "__index");
 
    // The Lua stack at this point looks like this:
    //     
    //     1| metatable "luaL_Foo"   |-1
 
    // The luaL_Foo metatable now has the following fields
    //     - __gc
    //     - __index
    //     - add
    //     - new
 
    // Now we use setglobal to officially expose the luaL_Foo metatable 
    // to Lua. And we use the name "Foo".
    //
    // This allows Lua scripts to _override_ the metatable of Foo.
    // For high security code this may not be called for but 
    // we'll do this to get greater flexibility.
    //lua_setglobal(l, "Foo");
}

int test()
{
    lua_State * l = luaL_newstate();
    luaL_openlibs(l);
	luaL_RegisterRequireFunc( l, "Foo", lua_foorequire );
    RegisterFoo(l);

    int erred = luaL_dofile(l, "fun.lua");
    if(erred)
        std::cout << "Lua error: " << luaL_checkstring(l, -1) << std::endl;

    lua_close(l);

    return 0;
}
#endif
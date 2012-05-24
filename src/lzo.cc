/*

Thanks to Peteris Krumins for the node-base64 implementation I used as a template to implement this:
https://github.com/pkrumins/node-base64

And of course to Markus F.X.J. Oberhumer for releasing the mini LZO library (GPL v2), which this module wraps:
http://www.oberhumer.com/opensource/lzo/


*/

#include <node_buffer.h>

#include "minilzo/minilzo.h"

using namespace v8;
using namespace node;

static Handle<Value>
VException(const char *msg) {
    HandleScope scope;
    return ThrowException(Exception::Error(String::New(msg)));
}


#define HEAP_ALLOC(var,size) \
    lzo_align_t __LZO_MMODEL var [ ((size) + (sizeof(lzo_align_t) - 1)) / sizeof(lzo_align_t) ]

static HEAP_ALLOC(wrkmem, LZO1X_1_MEM_COMPRESS);

int
compress(const unsigned char *input, unsigned char *output, lzo_uint in_len, lzo_uint& out_len)
{

    int r;

    if (lzo_init() != LZO_E_OK){
    	printf("lzo cannot initialize: this usually indicates a compiler error");
    	throw "lzo cannot initialize: this usually indicates a compiler error";
    }

	r = lzo1x_1_compress(input,in_len,output,&out_len,wrkmem);
	
    if (r == LZO_E_OK){
    	return 0;
    }else{
    	printf("lzo failed to compress");
    	throw "lzo failed to compress";
    	return -1;
    }
}

int
decompress(const unsigned char *input, unsigned char *output, lzo_uint in_len, lzo_uint& out_len)
{
	int r;

    if (lzo_init() != LZO_E_OK){
    	printf("lzo cannot initialize: this usually indicates a compiler error");
    	throw "lzo cannot initialize: this usually indicates a compiler error";
    }

    r = lzo1x_decompress(input,in_len,output,&out_len,NULL);

    if (r == LZO_E_OK){
    	return 0;
    }else{
    	if(r == LZO_E_OUTPUT_OVERRUN){
    		printf("\nlzo error: LZO_E_OUTPUT_OVERRUN\n");
			//throw "lzo failed to decompress";
    	}else if(r ==  LZO_E_INPUT_NOT_CONSUMED){
    		printf("\nlzo error: LZO_E_INPUT_NOT_CONSUMED\n");
    		printf("This usually indicates that your compressed data\ndoes not fill the entire buffer you provided.\n\n");
			//throw "lzo failed to decompress";
    	}else{
			printf("\nERROR: lzo failed to decompress: %i\n", r);
			//throw "lzo failed to decompress";
		}
		return -1;
    }
}

Handle<Value>
lzo_compress(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() == 2){

		if (!Buffer::HasInstance(args[0]))
		    return VException("Argument 1 should be a buffer");
		if (!Buffer::HasInstance(args[1]))
		    return VException("Argument 2 should be a buffer");

		v8::Handle<v8::Object> inputBuffer = args[0]->ToObject();
		v8::Handle<v8::Object> outputBuffer = args[1]->ToObject();
		
		lzo_uint output_len = Buffer::Length(outputBuffer);//not sure if output_len is read before it is written...
		
		lzo_uint input_len = Buffer::Length(inputBuffer);
		
		compress(
			(unsigned char *) Buffer::Data(inputBuffer), 
			(unsigned char *) Buffer::Data(outputBuffer), 
			input_len,
			output_len
		);
		
		return scope.Close(Integer::New(output_len));
	}else if(args.Length() == 5){

		if (!Buffer::HasInstance(args[0]))
		    return VException("Argument 1 should be a buffer");
		if (!Buffer::HasInstance(args[3]))
		    return VException("Argument 4 should be a buffer");

		v8::Handle<v8::Object> inputBuffer = args[0]->ToObject();
		int srcOff = args[1]->Int32Value();
		int srcLen = args[2]->Int32Value();
		v8::Handle<v8::Object> outputBuffer = args[3]->ToObject();
		int outOff = args[4]->Int32Value();
		
		lzo_uint output_len = Buffer::Length(outputBuffer) - outOff;//looks like output_len is read before it is written...
		
		lzo_uint input_len = srcLen;
		
		compress(
			((unsigned char *) Buffer::Data(inputBuffer)) + srcOff, 
			((unsigned char *) Buffer::Data(outputBuffer)) + outOff,
			input_len,
			output_len
		);

		return scope.Close(Integer::New(output_len));		    
	}else{
		return VException("Two or five arguments should be provided: compress(uncompressedBuffer, outputBuffer) or compress(uncompressedBuffer, srcOff, srcLen, outputBuffer, outOff)");
	}
}

Handle<Value>
lzo_decompress(const Arguments &args)
{

	HandleScope scope;

    if (args.Length() == 2){

		if (!Buffer::HasInstance(args[0]))
		    return VException("Argument 1 should be a buffer");
		if (!Buffer::HasInstance(args[1]))
		    return VException("Argument 2 should be a buffer");

		v8::Handle<v8::Object> inputBuffer = args[0]->ToObject();
		v8::Handle<v8::Object> outputBuffer = args[1]->ToObject();
		
		lzo_uint output_len = Buffer::Length(outputBuffer);//not sure if output_len is read before it is written...
		
		decompress(
			(unsigned char *) Buffer::Data(inputBuffer), 
			(unsigned char *) Buffer::Data(outputBuffer), 
			Buffer::Length(inputBuffer),
			output_len
		);
	}else if(args.Length() == 5){
		if (!Buffer::HasInstance(args[0]))
		    return VException("Argument 1 should be a buffer");
		if (!Buffer::HasInstance(args[3]))
		    return VException("Argument 4 should be a buffer");

		v8::Handle<v8::Object> inputBuffer = args[0]->ToObject();
		int srcOff = args[1]->Int32Value();
		int srcLen = args[2]->Int32Value();
		v8::Handle<v8::Object> outputBuffer = args[3]->ToObject();
		int outOff = args[4]->Int32Value();
		
		lzo_uint output_len = 0;//not read before written, I guess
		
		decompress(
			((unsigned char *) Buffer::Data(inputBuffer)) + srcOff, 
			((unsigned char *) Buffer::Data(outputBuffer)) + outOff, 
			srcLen,
			output_len
		);
	}else{
        return VException("Two or five arguments should be provided: decompress(compressedBuffer, outputBuffer) or decompress(compressedBuffer, srcOff, srcLen, outputBuffer, outOff)");
	}    
    return Undefined();
}

extern "C" void init (Handle<Object> target)
{
    HandleScope scope;
    target->Set(String::New("compress"), FunctionTemplate::New(lzo_compress)->GetFunction());
    target->Set(String::New("decompress"), FunctionTemplate::New(lzo_decompress)->GetFunction());
}
